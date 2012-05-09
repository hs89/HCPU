#include <iostream>
#include <sstream>
#include <deque>
#include "Assembler.h"
#include "Core.h"
#include "CommLink.h"
using namespace std;

string main_code = "Assembly/muldivloop.asm";
string machine_code_file = "MACHINE_CODE";

void setDebug(Core&,bool);
int PCoreInUse[4] = {0,0,0,0};



int main(int argc, char * argv[])
{

    cout<<"Assembled Successfully -- type ppm to view program memory"<<endl;
    Assembler p_asm = Assembler("opcodes"); 
    Assembler s_asm = Assembler("opcodes");
    p_asm.Assemble("Assembly/procnode.asm","PNODE_MC");
    s_asm.Assemble("Assembly/supernode.asm","SNODE_MC");
    
    Core SP = Core("SNODE_MC","sp_rfile",s_asm.TISRA, s_asm.RISRA);
    Core P0 = Core("PNODE_MC","pn0_rfile",p_asm.TISRA, p_asm.RISRA);
    Core P1 = Core("PNODE_MC","pn1_rfile",p_asm.TISRA, p_asm.RISRA);
    Core P2 = Core("PNODE_MC","pn2_rfile",p_asm.TISRA, p_asm.RISRA);
    Core P3 = Core("PNODE_MC","pn3_rfile",p_asm.TISRA, p_asm.RISRA);
    CommLink SPP0 = CommLink(SP,P0,0x04,0x00,0x09,0x05,0x0A,0x0B);
    
    string command;
    cout<<"Welcome to HCPU -- type help or ? for a command listing"<<endl;
    setDebug(SP,true);
    setDebug(P0,false);
    setDebug(P1,false);
    setDebug(P2,false);
    setDebug(P3,false);
    int stop;
    int running = 0;
    while(stop != 1)
    {
         if(!running)
         {
             cout<<"$ ";
             cin>>command;
         }
         if(command == "c")
         {
             running = 1;
             command = "n";
         }
         stop = SP.clockCore(command);
    }
    system("pause");
      
    return 0;
}

void setDebug(Core& c, bool debug)
{
    c.PIPE_DEBUG = debug;
    c.PRINT_FINAL_STATE = debug;
    c.PRINT_PC = debug;
    c.CONTROL_DEBUG = debug;
    c.INTERRUPT_DEBUG = debug;
    c.FLAG_DEBUG = debug;
    c.REGISTER_DEBUG = debug;
    c.DEBUG_DFWD = debug;
}
