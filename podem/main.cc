#include <iostream>
#include <ctime>
#include "circuit.h"
#include "GetLongOpt.h"
#include "ReadPattern.h"

#include<sys/types.h>
#include<unistd.h>

using namespace std;

// All defined in readcircuit.l
extern char* yytext;
extern FILE *yyin;
extern CIRCUIT Circuit;
extern int yyparse (void);
extern bool ParseError;

extern void Interactive();

GetLongOpt option;

int SetupOption(int argc, char ** argv)
{
    option.usage("[options] input_circuit_file");
    option.enroll("help", GetLongOpt::NoValue,
            "print this help summary", 0);
    option.enroll("logicsim", GetLongOpt::NoValue,
            "run logic simulation", 0);
    option.enroll("mod_logicsim", GetLongOpt::NoValue,
            "run modified logic simulation", 0);
    option.enroll("plogicsim", GetLongOpt::NoValue,
            "run parallel logic simulation", 0);
    option.enroll("fsim", GetLongOpt::NoValue,
            "run stuck-at fault simulation", 0);
    option.enroll("stfsim", GetLongOpt::NoValue,
            "run single pattern single transition-fault simulation", 0);
    option.enroll("transition", GetLongOpt::NoValue,
            "run transition-fault ATPG", 0);
    option.enroll("input", GetLongOpt::MandatoryValue,
            "set the input pattern file", 0);
    option.enroll("output", GetLongOpt::MandatoryValue,
            "set the output file", 0);
    option.enroll("bt", GetLongOpt::OptionalValue,
            "set the backtrack limit", 0);
    // Assignment 0
    option.enroll("ass0", GetLongOpt::NoValue,
            "print circuit info", 0);
    // Assignment 1
    option.enroll("path", GetLongOpt::NoValue,
            "list and count all possible paths connecting the given PI and PO", 0);
    option.enroll("start", GetLongOpt::MandatoryValue,
            "set the starting PI", 0);
    option.enroll("end", GetLongOpt::MandatoryValue,
            "set the ending PO", 0);
    // Assignment 2
    option.enroll("pattern", GetLongOpt::NoValue,
            "gen pattern generation", 0);
    option.enroll("num", GetLongOpt::MandatoryValue,
            "set the number of patterns to generate", 0);
    option.enroll("unknown", GetLongOpt::NoValue,
            "generate pattern with unknown", 0);
    // Assignment 3
    option.enroll("simulator", GetLongOpt::MandatoryValue,
            "run simulator", 0);
    // Assignment 4
    option.enroll("check_point", GetLongOpt::NoValue,
            "generate check point fault list", 0);
    option.enroll("bridging", GetLongOpt::NoValue,
            "generate bridging fault list", 0);
    // Assignment 5
    option.enroll("bridging_fsim", GetLongOpt::NoValue,
            "run bridging fault simulation", 0);
    // Assignment 6
    option.enroll("c17_proc", GetLongOpt::NoValue,
            "print c17 procedure", 0);
    option.enroll("random_pattern", GetLongOpt::NoValue,
            "generate random pattern", 0);
    option.enroll("bridging_atpg", GetLongOpt::NoValue,
            "run bridging fault ATPG", 0);
    int optind = option.parse(argc, argv);
    if ( optind < 1 ) { exit(0); }
    if ( option.retrieve("help") ) {
        option.usage();
        exit(0);
    }
    return optind;
}

void MemoryUsage()
{
    cout << "----------------- Memory usage ------------------" << endl;
    int pid=(int) getpid();
    cout << "pid: " << pid << endl;
    cout << "size | resident | shared | text | lib | data | data" << endl;
    char buf[1024];
    sprintf(buf, "cat /proc/%d/statm",pid);
    int ret = system(buf);
    if(ret == -1) {
        cout << "Error in system call" << endl;
    }
    cout << "-------------------------------------------------" << endl;
}

int main(int argc, char ** argv)
{
    int optind = SetupOption(argc, argv);
    clock_t time_init, time_end;
    time_init = clock();
    //Setup File
    if (optind < argc) {
        if ((yyin = fopen(argv[optind], "r")) == NULL) {
            cout << "Can't open circuit file: " << argv[optind] << endl;
            exit( -1);
        }
        else {
            string circuit_name = argv[optind];
            string::size_type idx = circuit_name.rfind('/');
            if (idx != string::npos) { circuit_name = circuit_name.substr(idx+1); }
            idx = circuit_name.find(".bench");
            if (idx != string::npos) { circuit_name = circuit_name.substr(0,idx); }
            Circuit.SetName(circuit_name);
        }
    }
    else {
        cout << "Input circuit file missing" << endl;
        option.usage();
        return -1;
    }
    cout << "Start parsing input file\n";
    yyparse();
    if (ParseError) {
        cerr << "Please correct error and try Again.\n";
        return -1;
    }
    fclose(yyin);
    Circuit.FanoutList();
    Circuit.SetupIO_ID();
    Circuit.Levelize();
    Circuit.Check_Levelization();
    Circuit.InitializeQueue();

    if (option.retrieve("logicsim")) {
        //logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.LogicSimVectors();
        // MemoryUsage();
    }
    else if (option.retrieve("mod_logicsim")) {
        //modified logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.ModLogicSimVectors();
        // MemoryUsage();
    }
    else if (option.retrieve("plogicsim")) {
        //parallel logic simulator
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.ParallelLogicSimVectors();
    }
    else if (option.retrieve("stfsim")) {
        //single pattern single transition-fault simulation
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.TFaultSimVectors();
    }
    else if (option.retrieve("transition")) {
        Circuit.MarkOutputGate();
        Circuit.GenerateAllTFaultList();
        Circuit.SortFaninByLevel();
        if (option.retrieve("bt")) {
            Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
        }
        Circuit.TFAtpg();
    }
    // Assignment 0
    else if (option.retrieve("ass0")) {
        cout << "run assignment 0" << endl;
        Circuit.PrintAssignment0();
    }
    // Assignment 1
    else if (option.retrieve("path")) {
        Circuit.MarkOutputGate();
        cout << "List all possible paths connecting " << option.retrieve("start") << " and " << option.retrieve("end") << endl;
        Circuit.GenerateAllPaths(option.retrieve("start"), option.retrieve("end"));
    }
    // Assignment 2
    else if(option.retrieve("pattern")) {
        cout << "run pattern generation" << endl;
        Circuit.GeneratePattern(option.retrieve("output"), stoi(option.retrieve("num")));
    }
    // Assignment 3
    else if(option.retrieve("simulator")) {
        cout << "run simulator" << endl;
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.GenerateCompiledCode();
    }
    // Assignment 4
    else if(option.retrieve("bridging")) {
        cout << "run bridging fault list" << endl;
        Circuit.GenerateBridgingFaultList();
    }
    // Assignment 5
    else if(option.retrieve("bridging_fsim")) {
        cout << "run bridging fault simulation" << endl;
        Circuit.GenerateBridgingFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.BridgeFaultSimVectors();
    }
    // Assignment 6_c
    else if(option.retrieve("c17_proc")) {
        cout << "print procedure for c17.bench with net17 stuck-at-0 and n60 stuck-at-1" << endl;
        Circuit.SetPrintPropagateTree(true);
        Circuit.GenerateC17FaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        if (option.retrieve("bt")) {
            Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
        }
        Circuit.Atpg();
    }
    // Assignment 6_d
    else if(option.retrieve("random_pattern")) {
        cout << "run random pattern generation" << endl;
        Circuit.GenerateAllFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        Circuit.GeneratePattern(option.retrieve("output"), 1000);
        Circuit.InitPattern(option.retrieve("output"));

        int counter = 0;
        while(Circuit.GetCoverage() < 90.0) {    
            Circuit.FaultSimVectors();
            counter++;
            // cout << "Coverage = " << Circuit.GetCoverage() << "%" << endl;
            if(Circuit.GetCoverage() >= 90.0) {
                cout << "Coverage reached 90% with pattern num = " << counter << endl;
                break;
            }
            if(counter > 1000) {
                cout << "Can't reach 100% coverage with random pattern (" << Circuit.GetCoverage() << "/90)" << endl;
                break;
            }
        }
        if (option.retrieve("bt")) {
            Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
        }
        Circuit.Atpg();
    } 
    // Assignment 6_e
    else if(option.retrieve("bridging_atpg")) {
        cout << "run bridging fault ATPG" << endl;
        Circuit.GenerateBridgingFaultList();
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        if (option.retrieve("bt")) {
            Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
        }
        //bridging fualt ATPG
        Circuit.BridgeFaultAtpg();
    }
    else {
        // Assignment 4 generate checkpoint fault list and print percentage of fault
        // Assignment 5 use check_point fault list to run ATPG
        // Assignment 6_a compare different backtrack limit on ATPG
        // Assignment 6_b compare check_point fault list 和 all fault list on ATPG(用checkpoint fault list跑ATPG，然后再用all fault list跑fsim)
        if(option.retrieve("check_point")){
            Circuit.GenerateCheckPointFaultList();
            if(!option.retrieve("output")){
                Circuit.GenerateAllFaultList();
                Circuit.PercentageOfFault();
                goto end;
            }
        }
        else{
            Circuit.GenerateAllFaultList();
        }
        Circuit.SortFaninByLevel();
        Circuit.MarkOutputGate();
        if (option.retrieve("fsim")) {
            //stuck-at fault simulator
            Circuit.InitPattern(option.retrieve("input"));
            Circuit.FaultSimVectors();
        }
        else {
            if (option.retrieve("bt")) {
                Circuit.SetBackTrackLimit(atoi(option.retrieve("bt")));
            }
            //stuck-at fualt ATPG
            Circuit.Atpg();
        }
    }
    end:
    time_end = clock();
    cout << "total CPU time = " << double(time_end - time_init)/CLOCKS_PER_SEC << endl;
    cout << endl;
    
    return 0;
}
