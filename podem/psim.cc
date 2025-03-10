#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
using namespace std;

extern GetLongOpt option;

// Event-driven Parallel Pattern Logic simulation
void CIRCUIT::ParallelLogicSimVectors()
{
    cout << "Run Parallel Logic simulation" << endl;
    patternoutput.open(option.retrieve("output"), ios::out);
    unsigned pattern_num(0);
    unsigned pattern_idx(0);
    evaluation_num = 0;
    while(!Pattern.eof()){ 
	for(pattern_idx=0; pattern_idx<PatternNum; pattern_idx++){
	    if(!Pattern.eof()){ 
	        ++pattern_num;
	        Pattern.ReadNextPattern(pattern_idx);
	    }
	    else break;
	}
	ScheduleAllPIs();
	ParallelLogicSim();
	PrintParallelIOs(pattern_idx);
    }
    patternoutput.close();
    cout << "========================================" << endl;
    cout << "Circular Name: " << Name << endl;
    cout << "Total number of patterns: " << pattern_num << endl;
    cout << "Packet size: " << PatternNum << endl;
    cout << "Total number of evaluations: " << evaluation_num << endl;
    double avg_eval = (double)evaluation_num / (double)pattern_num;
    cout << "Average number of evaluations per pattern: " << avg_eval << endl;
    double avg_eval_per_gate = avg_eval / (double)No_Gate() * 100;
    cout << "Percentage of average evaluations over total gate: " << avg_eval_per_gate << "%" << endl;
    cout << "========================================" << endl;
}

//Assign next input pattern to PI's idx'th bits
void PATTERN::ReadNextPattern(unsigned idx)
{
    char V;
    for (int i = 0; i < no_pi_infile; i++) {
        patterninput >> V;
        if (V == '0') {
            inlist[i]->ResetWireValue(0, idx);
            inlist[i]->ResetWireValue(1, idx);
        }
        else if (V == '1') {
            inlist[i]->SetWireValue(0, idx);
            inlist[i]->SetWireValue(1, idx);
        }
        else if (V == 'X') {
            inlist[i]->SetWireValue(0, idx);
            inlist[i]->ResetWireValue(1, idx);
        }
    }
    //Take care of newline to force eof() function correctly
    patterninput >> V;
    if (!patterninput.eof()) patterninput.unget();
    return;
}

//Simulate PatternNum vectors
void CIRCUIT::ParallelLogicSim()
{
    GATE* gptr;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            ParallelEvaluate(gptr);
        }
    }
    return;
}

//Evaluate parallel value of gptr
void CIRCUIT::ParallelEvaluate(GATEPTR gptr)
{
    register unsigned i;
    bitset<PatternNum> new_value1(gptr->Fanin(0)->GetValue1());
    bitset<PatternNum> new_value2(gptr->Fanin(0)->GetValue2());
    evaluation_num += gptr->No_Fanin();
    switch(gptr->GetFunction()) {
        case G_AND:
        case G_NAND:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 &= gptr->Fanin(i)->GetValue1();
                new_value2 &= gptr->Fanin(i)->GetValue2();
            }
            break;
        case G_OR:
        case G_NOR:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                new_value1 |= gptr->Fanin(i)->GetValue1();
                new_value2 |= gptr->Fanin(i)->GetValue2();
            }
            break;
        default: break;
    } 
    //swap new_value1 and new_value2 to avoid unknown value masked
    if (gptr->Is_Inversion()) {
        new_value1.flip(); new_value2.flip();
        bitset<PatternNum> value(new_value1);
        new_value1 = new_value2; new_value2 = value;
    }
    if (gptr->GetValue1() != new_value1 || gptr->GetValue2() != new_value2) {
        gptr->SetValue1(new_value1);
        gptr->SetValue2(new_value2);
        ScheduleFanout(gptr);
    }
    return;
}

void CIRCUIT::PrintParallelIOs(unsigned idx)
{
    register unsigned i;
    for (unsigned j=0; j<idx; j++){
	    for (i = 0;i<No_PI();++i) { 
		    if(PIGate(i)->GetWireValue(0, j)==0){ 
			   if(PIGate(i)->GetWireValue(1, j)==1){
	    			// cout << "F";
                    patternoutput << "F";
			   }
			   else{
                   // cout << "0";
                   patternoutput << "0";
                } 
		    }
		    else{
			   if(PIGate(i)->GetWireValue(1, j)==1){
	    			// cout << "1";
                    patternoutput << "1";
			   }
			   else {
                    // cout << "X";
                    patternoutput << "2";
               }
		    }

	    }
	    // cout << " ";
        patternoutput << " ";
	    for (i = 0;i<No_PO();++i) { 
		    if(POGate(i)->GetWireValue(0, j)==0){ 
			   if(POGate(i)->GetWireValue(1, j)==1){
	    			// cout << "F";
                    patternoutput << "F";
			   }
			   else {
                    // cout << "0";
                    patternoutput << "0";
               }
		    }
		    else{
			   if(POGate(i)->GetWireValue(1, j)==1){
	    			// cout << "1";
                    patternoutput << "1";
			   }
			   else {
                    // cout << "X";
                    patternoutput << "2";
               }
		    }
	    }
	    // cout << endl;
        patternoutput << endl;
    }
    return;
}

void CIRCUIT::ScheduleAllPIs()
{
    for (unsigned i = 0;i < No_PI();i++) {
        ScheduleFanout(PIGate(i));
    }
    return;
}
