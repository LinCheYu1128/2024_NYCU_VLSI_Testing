#include <iostream> 
//#include <alg.h>
#include "circuit.h"
#include "GetLongOpt.h"
#include <stack>
#include <list>
using namespace std;

extern GetLongOpt option;

// For assignment 0
void CIRCUIT::PrintAssignment0()
{
    int total_fanout = 0;
    int total_signal_net = 0;
    int No_branch = 0;
    int No_stem = 0;

    for (const auto &item : Netlist){
        // cout << "Gate " << item->GetID() << " " << item->GetName() << " "<< item->GetFunction() << endl;
        // cout << "  Fanin: " << item->No_Fanin() << " Fanout: " << item->No_Fanout() << endl << endl;
        if(item->No_Fanout()>1){
            total_signal_net += item->No_Fanout() + 1;
            No_branch += item->No_Fanout();
            No_stem ++;
        }
        else{
            total_signal_net += item->No_Fanout();
        }
        total_fanout += item->No_Fanout();
    }

    cout << "=================== Assignment 0 =====================" << endl;
    cout << "Number of inputs: " << No_PI() << endl;
    cout << "Number of outputs: " << No_PO() << endl;
    cout << "Number of gates: " << No_Total_Gate() << endl;
    cout << "   Number of PI: " << No_PI() << endl;
    cout << "   Number of PO: " << No_PO() << endl;
    cout << "   Number of PPI: " << No_PPI() << endl;
    cout << "   Number of PPO: " << No_PPO() << endl;
    cout << "   Number of NOT: " << No_NOT_Gate() << endl;
    cout << "   Number of AND: " << No_AND_Gate() << endl;
    cout << "   Number of NAND: " << No_NAND_Gate() << endl;
    cout << "   Number of OR: " << No_OR_Gate() << endl;
    cout << "   Number of NOR: " << No_NOR_Gate() << endl;
    cout << "   Number of DFF: " << No_PPI() << endl;
    cout << "   Number of BUF: " << No_BUF_Gate() << endl;
    cout << "Number of flip-flops: " << No_PPI() << endl;
    cout << "Total number of signal nets: " << total_signal_net << endl;
    cout << "Number of branch nets " << No_branch << endl;
    cout << "Number of stem nets: " << No_stem << endl;
    cout << "Average number of fanouts of each gate: " << (float)total_fanout/No_Gate() << endl;
    cout << "====================================================" << endl;
}

void CIRCUIT::reverseTraversal(vector<bool> &canReach, GATE* end_gate)
{
    vector<GATE*> stack;
    stack.push_back(end_gate);
    canReach.at(end_gate->GetID()) = true;

    while (!stack.empty()){
        GATE* current_gate = stack.back();
        stack.pop_back();
        for (unsigned i = 0; i < current_gate->No_Fanin(); i++){
            GATE* next_gate = current_gate->Fanin(i);
            if(!canReach.at(next_gate->GetID())){
                canReach.at(next_gate->GetID()) = true;
                stack.push_back(next_gate);
            }
        }
    }
}

void CIRCUIT::GenerateAllPaths(string start, string end)
{
    GATE* start_gate = FindGate(start, "PI");
    if(start_gate == nullptr){
        cout << "Start gate not found" << endl;
        return;
    }
    // else{
    //     cout << "Start gate found: " << start_gate->GetName() << " (ID:" << start_gate->GetID() << ")" << endl;
    // }
    GATE* end_gate = FindGate(end, "PO");
    if(end_gate == nullptr){
        cout << "End gate not found" << endl;
        return;
    }
    // else{
    //     cout << "End gate found: " << end_gate->GetName() << " (ID:" << end_gate->GetID() << ")" << endl;
    // }
        
    // ***************************** iterative function to print all paths ********************** //

    vector<GATE*> path;
    path.push_back(start_gate);

    vector<pair<GATE*, vector<GATE*>>> stack; // DFS stack
    stack.push_back({start_gate, path});
    
    vector<bool> canReach; // to check the gate have the path or not
    canReach.reserve(No_Gate());
    for (unsigned i = 0; i < No_Gate(); i++){
        canReach.push_back(false);
    }
    
    reverseTraversal(canReach, end_gate); // prunning the gates that can't reach the end gate

    GATE* next_gate;
    GATE* current_gate;
    int count = 0;
    
    while (!stack.empty()){               // DFS
        current_gate = stack.back().first;
        path = stack.back().second;
        stack.pop_back();

        if(current_gate == end_gate){
            count ++; 
            // cout << "Path found: " << count << endl;
            for (auto &gate : path){
                cout << gate->GetName() << " ";
            }
            cout << endl;
            
        }
        else{
            for(unsigned i = 0; i < current_gate->No_Fanout(); i++){
                if(canReach.at(current_gate->Fanout(i)->GetID())){
                    next_gate = current_gate->Fanout(i);
                    vector<GATE*> next_path = path;
                    next_path.push_back(next_gate);
                    stack.push_back({next_gate, next_path});
                }
            }
        }
        
    }
    
    cout << "The paths from " << start << " to " << end << ": " << count << endl;    
    
}

GATE* CIRCUIT::FindGate(string name, string type)
{
    if(type == "PI"){
        for (unsigned i = 0;i < No_PI();i++) {
            if (PIGate(i)->GetName() == name) {
                return PIGate(i);
            }
        }
    }
    else if(type == "PO"){
        for (unsigned i = 0;i < No_PO();i++) {
            if (POGate(i)->GetName() == name) {
                return POGate(i);
            }
        }
    }
    else if(type == "PPI"){
        for (unsigned i = 0;i < No_PPI();i++) {
            if (PPIGate(i)->GetName() == name) {
                return PPIGate(i);
            }
        }
    }
    else if(type == "PPO"){
        for (unsigned i = 0;i < No_PPO();i++) {
            if (PPOGate(i)->GetName() == name) {
                return PPOGate(i);
            }
        }
    }
    return nullptr;
}

void CIRCUIT::FanoutList()
{
    unsigned i = 0, j;
    GATE* gptr;
    for (;i < No_Gate();i++) {
        gptr = Gate(i);
        for (j = 0;j < gptr->No_Fanin();j++) {
            gptr->Fanin(j)->AddOutput_list(gptr);
        }

    }
}

void CIRCUIT::Levelize()
{
    list<GATE*> Queue;
    GATE* gptr;
    GATE* out;
    unsigned j = 0;
    for (unsigned i = 0;i < No_PI();i++) {
        gptr = PIGate(i);
        gptr->SetLevel(0);
        for (j = 0;j < gptr->No_Fanout();j++) {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI) {
                out->IncCount();
                if (out->GetCount() == out->No_Fanin()) {
                    out->SetLevel(1);
                    Queue.push_back(out);
                }
            }
        }
    }
    for (unsigned i = 0;i < No_PPI();i++) {
        gptr = PPIGate(i);
        gptr->SetLevel(0);
        for (j = 0;j < gptr->No_Fanout();j++) {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI) {
                out->IncCount();
                if (out->GetCount() ==
                        out->No_Fanin()) {
                    out->SetLevel(1);
                    Queue.push_back(out);
                }
            }
        }
    }
    int l1, l2;
    while (!Queue.empty()) {
        gptr = Queue.front();
        Queue.pop_front();
        l2 = gptr->GetLevel();
        for (j = 0;j < gptr->No_Fanout();j++) {
            out = gptr->Fanout(j);
            if (out->GetFunction() != G_PPI) {
                l1 = out->GetLevel();
                if (l1 <= l2)
                    out->SetLevel(l2 + 1);
                out->IncCount();
                if (out->GetCount() ==
                        out->No_Fanin()) {
                    Queue.push_back(out);
                }
            }
        }
    }
    for (unsigned i = 0;i < No_Gate();i++) {
        Gate(i)->ResetCount();
    }
}

void CIRCUIT::Check_Levelization()
{

    GATE* gptr;
    GATE* in;
    unsigned i, j;
    for (i = 0;i < No_Gate();i++) {
        gptr = Gate(i);
        if (gptr->GetFunction() == G_PI) {
            if (gptr->GetLevel() != 0) {
                cout << "Wrong Level for PI : " <<
                gptr->GetName() << endl;
                exit( -1);
            }
        }
        else if (gptr->GetFunction() == G_PPI) {
            if (gptr->GetLevel() != 0) {
                cout << "Wrong Level for PPI : " <<
                gptr->GetName() << endl;
                exit( -1);
            }
        }
        else {
            for (j = 0;j < gptr->No_Fanin();j++) {
                in = gptr->Fanin(j);
                if (in->GetLevel() >= gptr->GetLevel()) {
                    cout << "Wrong Level for: " <<
                    gptr->GetName() << '\t' <<
                    gptr->GetID() << '\t' <<
                    gptr->GetLevel() <<
                    " with fanin " <<
                    in->GetName() << '\t' <<
                    in->GetID() << '\t' <<
                    in->GetLevel() <<
                    endl;
                }
            }
        }
    }
}

void CIRCUIT::SetMaxLevel()
{
    for (unsigned i = 0;i < No_Gate();i++) {
        if (Gate(i)->GetLevel() > MaxLevel) {
            MaxLevel = Gate(i)->GetLevel();
        }
    }
}

//Setup the Gate ID and Inversion
//Setup the list of PI PPI PO PPO
void CIRCUIT::SetupIO_ID()
{
    unsigned i = 0;
    GATE* gptr;
    vector<GATE*>::iterator Circuit_ite = Netlist.begin();
    for (; Circuit_ite != Netlist.end();Circuit_ite++, i++) {
        gptr = (*Circuit_ite);
        gptr->SetID(i);
        switch (gptr->GetFunction()) {
            case G_PI: PIlist.push_back(gptr);
                break;
            case G_PO: POlist.push_back(gptr);
                break;
            case G_PPI: PPIlist.push_back(gptr);
                break;
            case G_PPO: PPOlist.push_back(gptr);
                break;
            case G_NOT: gptr->SetInversion(); NOTlist.push_back(gptr);
                break;
            case G_NAND: gptr->SetInversion(); NANDlist.push_back(gptr);
                break;
            case G_NOR: gptr->SetInversion(); NORlist.push_back(gptr);
                break;
            case G_AND: ANDlist.push_back(gptr);
                break;
            case G_OR: ORlist.push_back(gptr);
                break;
            case G_BUF: BUFlist.push_back(gptr);
                break;
            case G_DFF: DFFlist.push_back(gptr);
                break;
            default:
                break;
        }
    }
}
