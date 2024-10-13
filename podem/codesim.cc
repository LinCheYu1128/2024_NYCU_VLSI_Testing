#include <iostream>
#include "circuit.h"
#include "GetLongOpt.h" 
using namespace std;

extern GetLongOpt option;

void CIRCUIT::GenerateCompiledCode(){
    cout << "generate compiled code: " << option.retrieve("simulator") << endl;
    CodeSimulator.open(option.retrieve("simulator"), ios::out);
    CodeSimulator << "#include <iostream>" << endl;
    CodeSimulator << "#include <ctime>" << endl;
    CodeSimulator << "#include <bitset>" << endl;
    CodeSimulator << "#include <string>" << endl;
    CodeSimulator << "#include <fstream>" << endl << endl;
    CodeSimulator << "using namespace std;" << endl;
    CodeSimulator << "const unsigned PatternNum = 16;" << endl << endl;
    CodeSimulator << "void evaluate();" << endl;
    CodeSimulator << "void printIO(unsigned idx);" << endl << endl;

    for (unsigned i = 0; i < Netlist.size(); ++i) {
        CodeSimulator << "bitset<PatternNum> G_" << Netlist[i]->GetName() << "[2];" << endl;
    }
    CodeSimulator << "bitset<PatternNum> temp;" << endl;
    CodeSimulator << "ofstream fout(\"" << Name << ".out\",ios::out);" << endl << endl;
    CodeSimulator << "int main()\n{" << endl;
    CodeSimulator << "clock_t time_init, time_end;" << endl;
    CodeSimulator << "time_init = clock();" << endl;
    
    unsigned pattern_num(0);
    unsigned pattern_idx(0);
    while(!Pattern.eof()){ 
        for(pattern_idx=0; pattern_idx<PatternNum; pattern_idx++){
            if(!Pattern.eof()){ 
                ++pattern_num;
                Pattern.ReadNextPattern(pattern_idx);
            }
            else break;
        }
        ScheduleAllPIs();
        for (unsigned i = 0; i < PIlist.size(); ++i) {
            CodeSimulator << "G_" << PIlist[i]->GetName() << "[0] = " << int(long(PIlist[i]->GetWireValue(0).to_ulong())) << " ;" << endl;
            CodeSimulator << "G_" << PIlist[i]->GetName() << "[1] = " << int(long(PIlist[i]->GetWireValue(1).to_ulong())) << " ;" << endl;
        }
        CodeSimulator << endl << "evaluate();" << endl;
        CodeSimulator << "printIO(" << pattern_idx << ");" << endl << endl;
    }
    CodeSimulator << "time_end = clock();" << endl;
    CodeSimulator << "cout << \"Total CPU Time = \" << double(time_end - time_init)/CLOCKS_PER_SEC << endl;" << endl;
    CodeSimulator << "system(\"ps aux | grep a.out \");" << endl;
    CodeSimulator << "return 0;\n}" << endl;
    CodeSimulator << "void evaluate()\n{" << endl;
    
    CompileCodeLogicSim();
    
    CodeSimulator << "}\n" << "void printIO(unsigned idx)\n{" << endl;
    CodeSimulator << "for (unsigned j=0; j<idx; j++)" << endl;
    CodeSimulator << "{" ;
    for (unsigned i = 0; i < PIlist.size(); ++i) {
        CodeSimulator << "  if(G_" << PIlist[i]->GetName() << "[0][j]==0)" << endl;
        CodeSimulator << "  {" << endl;
        CodeSimulator << "      if(G_" << PIlist[i]->GetName() << "[1][j]==1)" << endl;
        CodeSimulator << "          fout<<\"F\";" << endl;
        CodeSimulator << "      else" << endl;
        CodeSimulator << "          fout<<\"0\";" << endl;
        CodeSimulator << "  }" << endl;
        CodeSimulator << "  else" << endl;
        CodeSimulator << "  {" << endl;
        CodeSimulator << "      if(G_" << PIlist[i]->GetName() << "[1][j]==1)" << endl;
        CodeSimulator << "          fout<<\"1\";" << endl;
        CodeSimulator << "      else" << endl;
        CodeSimulator << "          fout<<\"2\";" << endl;
        CodeSimulator << "  }" << endl;
    }
    CodeSimulator << "fout<<\" \";" << endl;
    for (unsigned i = 0; i < POlist.size(); ++i) {
        CodeSimulator << "  if(G_" << POlist[i]->GetName() << "[0][j]==0)" << endl;
        CodeSimulator << "  {" << endl;
        CodeSimulator << "      if(G_" << POlist[i]->GetName() << "[1][j]==1)" << endl;
        CodeSimulator << "          fout<<\"F\";" << endl;
        CodeSimulator << "      else" << endl;
        CodeSimulator << "          fout<<\"0\";" << endl;
        CodeSimulator << "  }" << endl;
        CodeSimulator << "  else" << endl;
        CodeSimulator << "  {" << endl;
        CodeSimulator << "      if(G_" << POlist[i]->GetName() << "[1][j]==1)" << endl;
        CodeSimulator << "          fout<<\"1\";" << endl;
        CodeSimulator << "      else" << endl;
        CodeSimulator << "      fout<<\"2\";" << endl;
        CodeSimulator << "  }" << endl;
    }
    CodeSimulator << "fout<<endl;" << endl;
    CodeSimulator << "}\n}" << endl;
    CodeSimulator.close();
}

void CIRCUIT::CompileCodeLogicSim(){
    GATE* gptr;
    for (unsigned i = 0;i <= MaxLevel;i++) {
        while (!Queue[i].empty()) {
            gptr = Queue[i].front();
            Queue[i].pop_front();
            gptr->ResetFlag(SCHEDULED);
            CompileCodeEvaluate(gptr);
        }
    }
    return;
}

void CIRCUIT::CompileCodeEvaluate(GATE* gptr){
    register unsigned i;
    CodeSimulator << "G_" << gptr->GetName() << "[0] = G_" << gptr->Fanin(0)->GetName() << "[0] ;" << endl;
    CodeSimulator << "G_" << gptr->GetName() << "[1] = G_" << gptr->Fanin(0)->GetName() << "[1] ;" << endl;
    switch (gptr->GetFunction()){
        case G_AND:
        case G_NAND:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                CodeSimulator << "G_" << gptr->GetName() << "[0] &= G_" << gptr->Fanin(i)->GetName() << "[0] ;" << endl;
                CodeSimulator << "G_" << gptr->GetName() << "[1] &= G_" << gptr->Fanin(i)->GetName() << "[1] ;" << endl;
            }
            break;
        case G_OR:
        case G_NOR:
            for (i = 1; i < gptr->No_Fanin(); ++i) {
                CodeSimulator << "G_" << gptr->GetName() << "[0] |= G_" << gptr->Fanin(i)->GetName() << "[0] ;" << endl;
                CodeSimulator << "G_" << gptr->GetName() << "[1] |= G_" << gptr->Fanin(i)->GetName() << "[1] ;" << endl;
            }
            break;
        default: break;
    } 
    if (gptr->Is_Inversion()) {
        CodeSimulator << "temp = G_" << gptr->GetName() << "[0] ;" << endl;
        CodeSimulator << "G_" << gptr->GetName() << "[0] = ~G_" << gptr->GetName() << "[1] ;" << endl;
        CodeSimulator << "G_" << gptr->GetName() << "[1] = ~temp ;" << endl;
    }
    ScheduleFanout(gptr);
    return;
}