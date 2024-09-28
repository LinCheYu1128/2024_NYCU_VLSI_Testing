/* Logic Simulator
 * Last update: 2006/09/20 */
#include <iostream>
#include "gate.h"
#include "circuit.h"
#include "ReadPattern.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

//do logic simulation for test patterns
void CIRCUIT::LogicSimVectors()
{
    cout << "Run logic simulation" << endl;
    patternoutput.open(option.retrieve("output"), ios::out);
    //read test patterns
    while (!Pattern.eof()) {
        Pattern.ReadNextPattern();
        SchedulePI();
        LogicSim();
        PrintIO("VALUE");
    }
    patternoutput.close();
    return;
}

//do event-driven logic simulation
void CIRCUIT::LogicSim()
{
    GATE* gptr;
    VALUE new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = Evaluate(gptr);
            if (new_value != gptr->GetValue()) {
                gptr->SetValue(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}

void CIRCUIT::ModLogicSimVectors()
{
    cout << "Run modified logic simulation" << endl;
    patternoutput.open(option.retrieve("output"), ios::out);
    //read test patterns
    while (!Pattern.eof()) {
        Pattern.ModReadNextPattern();
        SchedulePI();
        ModLogicSim();
        PrintIO("TRI");
    }
    patternoutput.close();
    return;
}

void CIRCUIT::ModLogicSim()
{
    GATE* gptr;
    TRI new_value;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            new_value = ModEvaluate(gptr);
            if (new_value != gptr->GetTri()) {
                gptr->SetTri(new_value);
                ScheduleFanout(gptr);
            }
        }
    }
    return;
}

//Used only in the first pattern
void CIRCUIT::SchedulePI()
{
    for (unsigned i = 0;i < No_PI();i++) {
        if (PIGate(i)->GetFlag(SCHEDULED)) {
            PIGate(i)->ResetFlag(SCHEDULED);
            ScheduleFanout(PIGate(i));
        }
    }
    return;
}

//schedule all fanouts of PPIs to Queue
void CIRCUIT::SchedulePPI()
{
    for (unsigned i = 0;i < No_PPI();i++) {
        if (PPIGate(i)->GetFlag(SCHEDULED)) {
            PPIGate(i)->ResetFlag(SCHEDULED);
            ScheduleFanout(PPIGate(i));
        }
    }
    return;
}

//set all PPI as 0
void CIRCUIT::SetPPIZero()
{
    GATE* gptr;
    for (unsigned i = 0;i < No_PPI();i++) {
        gptr = PPIGate(i);
        if (gptr->GetValue() != S0) {
            gptr->SetFlag(SCHEDULED);
            gptr->SetValue(S0);
        }
    }
    return;
}

//schedule all fanouts of gate to Queue
void CIRCUIT::ScheduleFanout(GATE* gptr)
{
    for (unsigned j = 0;j < gptr->No_Fanout();j++) {
        Schedule(gptr->Fanout(j));
    }
    return;
}

//initial Queue for logic simulation
void CIRCUIT::InitializeQueue()
{
    SetMaxLevel();
    Queue = new ListofGate[MaxLevel + 1];
    return;
}

//evaluate the output value of gate
VALUE CIRCUIT::Evaluate(GATEPTR gptr)
{
    GATEFUNC fun(gptr->GetFunction());
    VALUE cv(CV[fun]); //controling value
    VALUE value(gptr->Fanin(0)->GetValue());
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = AndTable[value][gptr->Fanin(i)->GetValue()];
            }
            break;
        case G_OR:
        case G_NOR:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = OrTable[value][gptr->Fanin(i)->GetValue()];
            }
            break;
        default: break;
    }
    //NAND, NOR and NOT
    if (gptr->Is_Inversion()) { value = NotTable[value]; }
    return value;
}

TRI CIRCUIT::ModEvaluate(GATEPTR gptr)
{
    GATEFUNC fun(gptr->GetFunction());
    TRI cv(CV_TRI[fun]); //controling value
    TRI value(gptr->Fanin(0)->GetTri());
    switch (fun) {
        case G_AND:
        case G_NAND:
            for (unsigned i = 1; i < gptr->No_Fanin() && value != cv;++i) {
                value = TRI(value & gptr->Fanin(i)->GetTri());
            }
            break;
        case G_OR:
        case G_NOR:
            for (unsigned i = 1;i<gptr->No_Fanin() && value != cv;++i) {
                value = TRI(value | gptr->Fanin(i)->GetTri());
            }
            break;
        default: break;
    }
    //NAND, NOR and NOT
    if (gptr->Is_Inversion()) { value = TRI(~value & 0b11); }
    return value;
}

extern GATE* NameToGate(string);

void PATTERN::Initialize(char* InFileName, int no_pi, string TAG)
{
    patterninput.open(InFileName, ios::in);
    if (!patterninput) {
        cout << "Unable to open input pattern file: " << InFileName << endl;
        exit( -1);
    }
    string piname;
    while (no_pi_infile < no_pi) {
        patterninput >> piname;
        if (piname == TAG) {
            patterninput >> piname;
            inlist.push_back(NameToGate(piname));
            no_pi_infile++;
        }
        else {
            cout << "Error in input pattern file at line "
                << no_pi_infile << endl;
            cout << "Maybe insufficient number of input\n";
            exit( -1);
        }
    }
    return;
}

//Assign next input pattern to PI
void PATTERN::ReadNextPattern()
{
    char V;
    for (int i = 0;i < no_pi_infile;i++) {
        patterninput >> V;
        if (V == '0') {
            if (inlist[i]->GetValue() != S0) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(S0);   
            }
        }
        else if (V == '1') {
            if (inlist[i]->GetValue() != S1) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(S1);
            }
        }
        else if (V == 'X') {
            if (inlist[i]->GetValue() != X) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetValue(X);
            }
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();
    return;
}

void PATTERN::ModReadNextPattern()
{
    char V;
    for (int i = 0;i < no_pi_infile;i++) {
        patterninput >> V;
        if (V == '0') {
            if (inlist[i]->GetTri() != LOW) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetTri(LOW);
            }
        }
        else if (V == '1') {
            if (inlist[i]->GetTri() != HIGH) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetTri(HIGH);
            }
        }
        else if (V == 'X') {
            if (inlist[i]->GetTri() != UNDEFINED0) {
                inlist[i]->SetFlag(SCHEDULED);
                inlist[i]->SetTri(UNDEFINED0);
            }
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();
    return;
}

void PATTERN::GenerateRandomPattern(unsigned num, vector<GATE*> GATElist, string TAG)
{
    patternoutput.open(option.retrieve("output"), ios::out);
    for (unsigned j = 0;j < GATElist.size();++j) {
        patternoutput << TAG << " " << GATElist[j]->GetName() << " ";
    }
    patternoutput << endl;
    for (unsigned i = 0;i < num; ++i) {
        for (unsigned j = 0;j < GATElist.size();++j) {
            int temp;
            if(option.retrieve("unknown")) temp = int(10.0 * (rand() / (RAND_MAX + 1.0)))%3;
            else temp = int(10.0 * (rand() / (RAND_MAX + 1.0)))%2;
            if (temp == 0) {
                patternoutput << "0";
            }
            else if (temp == 1) {
                patternoutput << "1";
            }
            else {
                patternoutput << "X";
            }
        }
        patternoutput << endl;
    }
    patternoutput.close();
    return;
}

void CIRCUIT::PrintIO(string type)
{
    register unsigned i;
    // write the output pattern
    if(type == "VALUE") {
        patternoutput << "PI: ";
        for (i = 0;i<No_PI();++i) { 
            if(PIGate(i)->GetValue() == S0) {
                cout << "0";
                patternoutput << "0";
            }
            else if(PIGate(i)->GetValue() == S1) {
                cout << "1";
                patternoutput << "1";
            }
            else {
                cout << "X";
                patternoutput << "X";
            }
        }
        cout << " ";
        patternoutput << " PO: ";
        for (i = 0;i<No_PO();++i) { 
            if(POGate(i)->GetValue() == S0) {
                cout << "0";
                patternoutput << "0";
            }
            else if(POGate(i)->GetValue() == S1) {
                cout << "1";
                patternoutput << "1";
            }
            else {
                cout << "X";
                patternoutput << "X";
            }
        }
        cout << endl;
        patternoutput << endl;
    }
    else if(type == "TRI") {
        patternoutput << "PI: ";
        for (i = 0;i<No_PI();++i) {
            if(PIGate(i)->GetTri() == LOW) {
                cout << "0";
                patternoutput << "0";
            }
            else if(PIGate(i)->GetTri() == HIGH) {
                cout << "1";
                patternoutput << "1";
            }
            else {
                cout << "X";
                patternoutput << "X";
            }
        }
        cout << " ";
        patternoutput << " PO: ";
        for (i = 0;i<No_PO();++i) { 
            if(POGate(i)->GetTri() == LOW) {
                cout << "0";
                patternoutput << "0";
            }
            else if(POGate(i)->GetTri() == HIGH) {
                cout << "1";
                patternoutput << "1";
            }
            else {
                cout << "X";
                patternoutput << "X";
            }
        }
        cout << endl;
        patternoutput << endl;
    }
    return;
}

