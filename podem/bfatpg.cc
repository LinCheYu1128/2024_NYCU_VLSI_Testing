/* bridging fault ATPG for combinational circuit
 * Last update: 2006/12/09 */
#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h"
#include <algorithm>   
using namespace std;

extern GetLongOpt option;

void CIRCUIT::GenerateBridgingFaultList()
{
    cout << "Generate bridging fault list" << endl;
    // Queue is used to store the gates in the same level
    for(auto gptr : Netlist){
        Queue[gptr->GetLevel()].push_back(gptr);
    }

    // Gnerate bridging fault list
    register unsigned i;
    FAULT *fptr;
    GATE* gptr;
    for(i = 0; i < MaxLevel; i++){
        while(Queue[i].size() > 1){
            gptr = Queue[i].front();
            Queue[i].pop_front();
            fptr = new FAULT(gptr, Queue[i].front(), S0);
            fptr->SetType(AND);
            BFlist.push_back(fptr);
            fptr = new FAULT(gptr, Queue[i].front(), S1);
            fptr->SetType(OR);
            BFlist.push_back(fptr);
        }
        Queue[i].pop_front();
    }

    //copy Flist to undetected Flist (for fault simulation)
    UBFlist = BFlist;
    cout << "Bridging fault number:" << UBFlist.size() <<endl;
    if(option.retrieve("bridging")){
        list<FAULT*>::iterator fite;
        bfaultoutput.open(option.retrieve("output"), ios::out);
        for (fite = UBFlist.begin(); fite != UBFlist.end();++fite) {
            fptr = *fite;
            if(fptr->GetType() == AND){
                bfaultoutput << "(" << fptr->GetInputGate()->GetName() << ", " << fptr->GetOutputGate()->GetName() << ", AND)" << endl;
                // cout << "(" << fptr->GetInputGate()->GetName() << ", " << fptr->GetOutputGate()->GetName() << ", AND)" << endl;
            }
            else{
                bfaultoutput << "(" << fptr->GetInputGate()->GetName() << " , " << fptr->GetOutputGate()->GetName() << ", OR)" << endl;
                // cout << "(" << fptr->GetInputGate()->GetName() << " , " << fptr->GetOutputGate()->GetName() << ", OR)" << endl;
            }
        }
        bfaultoutput.close();
    }
    
    return;
}

// run bridging fault ATPG
void CIRCUIT::BridgeFaultAtpg()
{
    cout << "Run bridging fault ATPG" << endl;
    unsigned i, total_backtrack_num(0);
    ATPG_STATUS status;
    FAULT* fptr;
    list<FAULT*>::iterator fite;
    
    //Prepare the output files
    ofstream OutputStrm;
    if (option.retrieve("output")){
        OutputStrm.open((char*)option.retrieve("output"),ios::out);
        if(!OutputStrm){
              cout << "Unable to open output file: "
                   << option.retrieve("output") << endl;
              cout << "Unsaved output!\n";
              exit(-1);
        }
    }
    
    if (option.retrieve("output")){
        for (i = 0;i<PIlist.size();++i) {
        OutputStrm << "PI " << PIlist[i]->GetName() << " ";
        }
        OutputStrm << endl;
    }    

    for (fite = BFlist.begin(); fite != BFlist.end();++fite) {
        fptr = *fite;
        if (fptr->GetStatus() == DETECTED) { continue; }
        //run podem algorithm
        status = BridgePodem(fptr, total_backtrack_num);
        switch (status) {
            case TRUE:
                fptr->SetStatus(DETECTED);
                ++pattern_num;
                //run fault simulation for fault dropping
                for (i = 0;i < PIlist.size();++i) { 
                    ScheduleFanout(PIlist[i]); 
                    if (option.retrieve("output")){ OutputStrm << PIlist[i]->GetValue();}
		        }
                if (option.retrieve("output")){ OutputStrm << endl;}
                for (i = PIlist.size();i<Netlist.size();++i) { Netlist[i]->SetValue(X); }
                LogicSim();
                BridgeFaultSim();
                break;
            case CONFLICT:
                fptr->SetStatus(REDUNDANT);
                break;
            case FALSE:
                fptr->SetStatus(ABORT);
                break;
        }
    } //end all faults

    //compute fault coverage
    unsigned total_num(0);
    unsigned abort_num(0), redundant_num(0), detected_num(0);
    unsigned eqv_abort_num(0), eqv_redundant_num(0), eqv_detected_num(0);
    for (fite = BFlist.begin();fite!=BFlist.end();++fite) {
        fptr = *fite;
        switch (fptr->GetStatus()) {
            case DETECTED:
                ++eqv_detected_num;
                detected_num += fptr->GetEqvFaultNum();
                break;
            case REDUNDANT:
                ++eqv_redundant_num;
                redundant_num += fptr->GetEqvFaultNum();
                break;
            case ABORT:
                ++eqv_abort_num;
                abort_num += fptr->GetEqvFaultNum();
                break;
            default:
                cerr << "Unknown fault type exists" << endl;
                break;
        }
    }
    total_num = detected_num + abort_num + redundant_num;

    cout.setf(ios::fixed);
    cout.precision(2);
    cout << "---------------------------------------" << endl;
    cout << "Test pattern number = " << pattern_num << endl;
    cout << "Total backtrack number = " << total_backtrack_num << endl;
    cout << "Backtrack limit = " << BackTrackLimit << endl;
    cout << "---------------------------------------" << endl;
    cout << "Total fault number = " << total_num << endl;
    cout << "Detected fault number = " << detected_num << endl;
    cout << "Undetected fault number = " << abort_num + redundant_num << endl;
    cout << "Abort fault number = " << abort_num << endl;
    cout << "Redundant fault number = " << redundant_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Total equivalent fault number = " << Flist.size() << endl;
    cout << "Equivalent detected fault number = " << eqv_detected_num << endl;
    cout << "Equivalent undetected fault number = " << eqv_abort_num + eqv_redundant_num << endl;
    cout << "Equivalent abort fault number = " << eqv_abort_num << endl;
    cout << "Equivalent redundant fault number = " << eqv_redundant_num << endl;
    cout << "---------------------------------------" << endl;
    cout << "Fault Coverge = " << 100*detected_num/double(total_num) << "%" << endl;
    cout << "Equivalent FC = " << 100*eqv_detected_num/double(BFlist.size()) << "%" << endl;
    cout << "Fault Efficiency = " << 100*detected_num/double(total_num - redundant_num) << "%" << endl;
    cout << "---------------------------------------" << endl;
    return;
}

//run Bridge PODEM for target fault
//TRUE: test pattern found
//CONFLICT: no test pattern
//FALSE: abort
ATPG_STATUS CIRCUIT::BridgePodem(FAULT* fptr, unsigned &total_backtrack_num)
{
    unsigned i, backtrack_num(0);
    GATEPTR pi_gptr(0), decision_gptr(0);
    ATPG_STATUS status;
    //set all values as unknown
    for (i = 0;i<Netlist.size();++i) { Netlist[i]->SetValue(X); }
    //mark propagate paths
    // cout << "fault: " << fptr->GetInputGate()->GetName() << " "<< fptr->GetOutputGate()->GetName() << " " << fptr->GetValue() << endl;
    MarkPropagateTree(fptr->GetOutputGate());
    //propagate fault free value
    status = SetUniqueImpliedValue(fptr);
    switch (status) {
        case TRUE:
            LogicSim();
            //inject faulty value
            if (BridgeFaultEvaluate(fptr)) {
                //forward implication
                ScheduleFanout(fptr->GetOutputGate());
                LogicSim();
            }
            //check if the fault has propagated to PO
            if (!CheckTest()) { status = FALSE; }
            break;
        case CONFLICT:
            status = CONFLICT;
            break;
        case FALSE: break;
    }
    while(backtrack_num < BackTrackLimit && status == FALSE) {
        //search possible PI decision
        pi_gptr = TestPossible(fptr);
        // cout << "  PI decision: " << pi_gptr->GetName() << " " << pi_gptr->PrintValue() << " Backtrack number: " << backtrack_num << endl;
        if (pi_gptr) { //decision found
            ScheduleFanout(pi_gptr);
            //push to decision tree
            GateStack.push_back(pi_gptr);
            decision_gptr = pi_gptr;
        }
        else { //backtrack previous decision
            while (!GateStack.empty() && !pi_gptr) {
                //all decision tried (1 and 0)
                if (decision_gptr->GetFlag(ALL_ASSIGNED)) {
                    decision_gptr->ResetFlag(ALL_ASSIGNED);
                    decision_gptr->SetValue(X);
                    ScheduleFanout(decision_gptr);
                    //remove decision from decision tree
                    GateStack.pop_back();
                    decision_gptr = GateStack.back();
                }
                //inverse current decision value
                else {
                    decision_gptr->InverseValue();
                    ScheduleFanout(decision_gptr);
                    decision_gptr->SetFlag(ALL_ASSIGNED);
                    ++backtrack_num;
                    pi_gptr = decision_gptr;
                }
            }
            //no other decision
            if (!pi_gptr) { status = CONFLICT; }
        }
        if (pi_gptr) {
            LogicSim();
            //fault injection
            if(BridgeFaultEvaluate(fptr)) {
                //forward implication
                ScheduleFanout(fptr->GetOutputGate());
                LogicSim();
            }

            if (CheckTest()) { status = TRUE; }
        }

    } //end while loop
    //clean ALL_ASSIGNED and MARKED flags
    list<GATEPTR>::iterator gite;
    for (gite = GateStack.begin();gite != GateStack.end();++gite) {
        (*gite)->ResetFlag(ALL_ASSIGNED);
    }
    for (gite = PropagateTree.begin();gite != PropagateTree.end();++gite) {
        (*gite)->ResetFlag(MARKED);
    }

    //clear stacks
    GateStack.clear(); PropagateTree.clear();
    
    //assign true values to PIs
    if (status ==  TRUE) {
		for (i = 0;i<PIlist.size();++i) {
		    switch (PIlist[i]->GetValue()) {
			case S1: break;
			case S0: break;
			case D: PIlist[i]->SetValue(S1); break;
			case B: PIlist[i]->SetValue(S0); break;
			case X: PIlist[i]->SetValue(VALUE(2.0 * rand()/(RAND_MAX + 1.0))); break;
			default: cerr << "Illigal value" << endl; break;
		    }
		}//end for all PI
    } //end status == TRUE

    total_backtrack_num += backtrack_num;
    return status;
}

bool CIRCUIT::BridgeFaultEvaluate(FAULT* fptr)
{
    GATEPTR igptr(fptr->GetInputGate());
    GATEPTR ogptr(fptr->GetOutputGate());
    //store value
    VALUE ivalue(igptr->GetValue());
    VALUE ovalue(ogptr->GetValue());

    //can not do fault injection
    if(fptr->GetType() == AND){ 
        if(ivalue == S0 && ovalue == S1){ igptr->SetValue(B); }
        else if(ivalue == S1 && ovalue == S0){ ogptr->SetValue(B); }
        else{ return false; }
    }
    else if(fptr->GetType() == OR){
        if(ivalue == S0 && ovalue == S1){ ogptr->SetValue(D); }
        else if(ivalue == S1 && ovalue == S0){ igptr->SetValue(D); }
        else{ return false; }
    }
    else{ return false; }

    if(igptr->GetFunction() == G_PI){
        igptr->SetValue(ivalue);
        return true;
    }
    if(ogptr->GetFunction() == G_PI){
        ogptr->SetValue(ovalue);
        return true;
    }
    else{
        VALUE value1(Evaluate(ogptr));
        VALUE value2(Evaluate(igptr));

        if (value1 != ogptr->GetValue() || value2 != igptr->GetValue()) {
            return true;
        }
    }
    return false;
}