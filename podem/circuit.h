#ifndef CIRCUIT_H
#define CIRCUIT_H
#include "fault.h"
#include "tfault.h"
#include "ReadPattern.h"
#include <stdlib.h>
#include <fstream>

typedef GATE* GATEPTR;

class CIRCUIT
{
    private:
        string Name;
        PATTERN Pattern;
        vector<GATE*> Netlist;
        vector<GATE*> PIlist; //store the gate indexes of PI
        vector<GATE*> POlist;
        vector<GATE*> PPIlist;
        vector<GATE*> PPOlist;
        // Gate list
        vector<GATE*> Gatelist;
        vector<GATE*> NOTlist;
        vector<GATE*> ANDlist;
        vector<GATE*> NANDlist;
        vector<GATE*> ORlist;
        vector<GATE*> NORlist;
        vector<GATE*> DFFlist;
        vector<GATE*> BUFlist;
        // pattern output
        ofstream patternoutput;

        list<FAULT*> Flist; //collapsing fault list
        list<FAULT*> UFlist; //undetected fault list
        list<TFAULT*> TFlist; //collapsing fault list
        list<TFAULT*> UTFlist; //undetected fault list
        list<FAULT*> CheckPointFList; //fault list for checkpoint
        list<FAULT*> UCheckPointFlist; //undetected fault list for checkpoint
        list<FAULT*> BFlist; //bridging fault list
        list<FAULT*> UBFlist; //undetected bridging fault list
        ofstream bfaultoutput;
        
        unsigned MaxLevel;
        unsigned BackTrackLimit; //backtrack limit for Podem
        typedef list<GATE*> ListofGate;
        typedef list<GATE*>::iterator ListofGateIte;
        ListofGate* Queue;
        ListofGate GateStack;
        ListofGate PropagateTree;
        ListofGateIte QueueIte;

        // For assignment 3
        unsigned evaluation_num;
        ofstream CodeSimulator;

        // For assignment 6
        bool print_pt;
        double coverage;
        unsigned pattern_num;
        
    public:
        //Initialize netlist
        CIRCUIT(): MaxLevel(0), BackTrackLimit(10000) {
            Netlist.reserve(32768);
            PIlist.reserve(128);
            POlist.reserve(512);
            PPIlist.reserve(2048);
            PPOlist.reserve(2048);
            coverage = 0;
            pattern_num = 0;
        }
        CIRCUIT(unsigned NO_GATE, unsigned NO_PI = 128, unsigned NO_PO = 512,
                unsigned NO_PPI = 2048, unsigned NO_PPO = 2048) {
            Netlist.reserve(NO_GATE);
            PIlist.reserve(NO_PI);
            POlist.reserve(NO_PO);
            PPIlist.reserve(NO_PPI);
            PPOlist.reserve(NO_PPO);
            coverage = 0;
            pattern_num = 0;
        }
        ~CIRCUIT() {
            for (unsigned i = 0;i<Netlist.size();++i) { delete Netlist[i]; }
            list<FAULT*>::iterator fite;
            for (fite = Flist.begin();fite!=Flist.end();++fite) { delete *fite; }
        }

        void AddGate(GATE* gptr) { Netlist.push_back(gptr); }
        void SetName(string n){ Name = n;}
        string GetName(){ return Name;}
        int GetMaxLevel(){ return MaxLevel;}
        void SetBackTrackLimit(unsigned bt) { BackTrackLimit = bt; }

        //Access the gates by indexes
        GATE* Gate(unsigned index) { return Netlist[index]; }
        GATE* PIGate(unsigned index) { return PIlist[index]; }
        GATE* POGate(unsigned index) { return POlist[index]; }
        GATE* PPIGate(unsigned index) { return PPIlist[index]; }
        GATE* PPOGate(unsigned index) { return PPOlist[index]; }
        unsigned No_Gate() { return Netlist.size(); }
        unsigned No_PI() { return PIlist.size(); }
        unsigned No_PO() { return POlist.size(); }
        unsigned No_PPI() { return PPIlist.size(); }
        unsigned No_PPO() { return PPOlist.size(); }
        unsigned No_Total_Gate() { return NOTlist.size() + ANDlist.size() + NANDlist.size() + ORlist.size() + NORlist.size(); }
        unsigned No_NOT_Gate() { return NOTlist.size(); }
        unsigned No_AND_Gate() { return ANDlist.size(); }
        unsigned No_NAND_Gate() { return NANDlist.size(); }
        unsigned No_OR_Gate() { return ORlist.size(); }
        unsigned No_NOR_Gate() { return NORlist.size(); }
        unsigned No_BUF_Gate() { return BUFlist.size(); }
        unsigned No_DFF_Gate() { return DFFlist.size(); }

        void InitPattern(const char *pattern) {
            Pattern.Initialize(const_cast<char *>(pattern), PIlist.size(), "PI");
        }

        void Schedule(GATE* gptr)
        {
            if (!gptr->GetFlag(SCHEDULED)) {
                gptr->SetFlag(SCHEDULED);
                Queue[gptr->GetLevel()].push_back(gptr);
            }
        }
        
        // For assignment 0
        void PrintAssignment0();

        // For assignment 1
        void GenerateAllPaths(string start, string end);
        void reverseTraversal(vector<bool> &canReach, GATE* end_gate);
        GATE* FindGate(string name, string type);

        // For assignment 2
        void GeneratePattern(const char *pattern, int num) {
            Pattern.GenerateRandomPattern(const_cast<char *>(pattern), num, PIlist, "PI");
        }

        // For assignment 6
        void SetPrintPropagateTree(bool b) { print_pt = b; }
        double GetCoverage() { return coverage; }

        //defined in circuit.cc
        void Levelize();
        void FanoutList();
        void Check_Levelization();
        void SetMaxLevel();
        void SetupIO_ID();

        //defined in sim.cc
        void SetPPIZero(); //Initialize PPI state
        void InitializeQueue();
        void ScheduleFanout(GATE*);
        void SchedulePI();
        void SchedulePPI();
        void LogicSimVectors();
        void LogicSim();
        void PrintIO();
        VALUE Evaluate(GATEPTR gptr);
        // For assignment 2 (modified logic simulator)
        void ModLogicSimVectors();
        void ModLogicSim();
        VALUE ModEvaluate(GATEPTR gptr);

        //defined in atpg.cc
        void GenerateAllFaultList();
        // For assignment 4
        void GenerateCheckPointFaultList();
        void PercentageOfFault(); 
        void GenerateBridgingFaultList();
        // For assignment 6
        void GenerateC17FaultList();
        void BridgeFaultAtpg();
        ATPG_STATUS BridgePodem(FAULT* fptr, unsigned &total_backtrack_num);
        bool BridgeFaultEvaluate(FAULT* fptr);

        void GenerateFaultList();
        void Atpg();
        void SortFaninByLevel();
        bool CheckTest();
        bool TraceUnknownPath(GATEPTR gptr);
        bool FaultEvaluate(FAULT* fptr);
        ATPG_STATUS Podem(FAULT* fptr, unsigned &total_backtrack_num);
        ATPG_STATUS SetUniqueImpliedValue(FAULT* fptr);
        ATPG_STATUS BackwardImply(GATEPTR gptr, VALUE value);
        GATEPTR FindPropagateGate();
        GATEPTR FindHardestControl(GATEPTR gptr);
        GATEPTR FindEasiestControl(GATEPTR gptr);
        GATEPTR FindPIAssignment(GATEPTR gptr, VALUE value);
        GATEPTR TestPossible(FAULT* fptr);
        void TraceDetectedStemFault(GATEPTR gptr, VALUE val);

        //defined in fsim.cc
        void MarkOutputGate();
        void MarkPropagateTree(GATEPTR gptr);
        void FaultSimVectors();
        void FaultSim();
        void FaultSimEvaluate(GATE* gptr);
        bool CheckFaultyGate(FAULT* fptr);
        void InjectFaultValue(GATEPTR gptr, unsigned idx,VALUE value);

        // defined in bfsim.cc
        // For assignment 5
        void BridgeFaultSimVectors();
        void BridgeFaultSim();
        

	//defined in psim.cc for parallel logic simulation
	void ParallelLogicSimVectors();
	void ParallelLogicSim();
	void ParallelEvaluate(GATEPTR gptr);
	void PrintParallelIOs(unsigned idx);
	void ScheduleAllPIs();

    // For assignment 3
    //defined in codesim.cc for compiled code simulation
    void GenerateCompiledCode();
    void CompileCodeLogicSim();
    void CompileCodeEvaluate(GATEPTR gptr);

	//defined in stfsim.cc for single pattern single transition-fault simulation
	void GenerateAllTFaultList();
	void TFaultSimVectors();
	void TFaultSim_t();
	void TFaultSim();
	bool CheckTFaultyGate(TFAULT* fptr);
	bool CheckTFaultyGate_t(TFAULT* fptr);
	VALUE Evaluate_t(GATEPTR gptr);
	void LogicSim_t();
        void PrintTransition();
        void PrintTransition_t();
        void PrintIO_t();

	//defined in tfatpg.cc for transition fault ATPG
	void TFAtpg();
	ATPG_STATUS Initialization(GATEPTR gptr, VALUE target, unsigned &total_backtrack_num);
	ATPG_STATUS BackwardImply_t(GATEPTR gptr, VALUE value);
	GATEPTR FindPIAssignment_t(GATEPTR gptr, VALUE value);
	GATEPTR FindEasiestControl_t(GATEPTR gptr);
	GATEPTR FindHardestControl_t(GATEPTR gptr);
};
#endif
