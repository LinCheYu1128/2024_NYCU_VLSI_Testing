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
            "set the output pattern file", 0);
    option.enroll("bt", GetLongOpt::OptionalValue,
            "set the backtrack limit", 0);
    option.enroll("ass0", GetLongOpt::NoValue,
            "print circuit info", 0);
    option.enroll("path", GetLongOpt::NoValue,
            "list and count all possible paths connecting the given PI and PO", 0);
    option.enroll("start", GetLongOpt::MandatoryValue,
            "set the starting PI", 0);
    option.enroll("end", GetLongOpt::MandatoryValue,
            "set the ending PO", 0);
    option.enroll("pattern", GetLongOpt::NoValue,
            "gen pattern generation", 0);
    option.enroll("num", GetLongOpt::MandatoryValue,
            "set the number of patterns to generate", 0);
    option.enroll("unknown", GetLongOpt::NoValue,
            "generate pattern with unknown", 0);
    option.enroll("simulator", GetLongOpt::MandatoryValue,
            "run simulator", 0);
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
    else if (option.retrieve("ass0")) {
        cout << "run assignment 0" << endl;
        Circuit.PrintAssignment0();
    }
    else if (option.retrieve("path")) {
        Circuit.MarkOutputGate();
        cout << "List all possible paths connecting " << option.retrieve("start") << " and " << option.retrieve("end") << endl;
        Circuit.GenerateAllPaths(option.retrieve("start"), option.retrieve("end"));
    }
    else if(option.retrieve("pattern")) {
        cout << "run pattern generation" << endl;
        Circuit.GeneratePattern(option.retrieve("input"), stoi(option.retrieve("num")));
    }
    else if(option.retrieve("simulator")) {
        cout << "run simulator" << endl;
        Circuit.InitPattern(option.retrieve("input"));
        Circuit.GenerateCompiledCode();
    }
    else {
        Circuit.GenerateAllFaultList();
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
    time_end = clock();
    cout << "total CPU time = " << double(time_end - time_init)/CLOCKS_PER_SEC << endl;
    cout << endl;
    
    return 0;
}
