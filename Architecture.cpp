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

    Assembler p_asm = Assembler("opcodes"); 
    Assembler s_asm = Assembler("opcodes");
    p_asm.Assemble("Assembly/procnode.asm","PNODE_MC");
    s_asm.Assemble("Assembly/supernode.asm","SNODE_MC");
    
    cout<<"\nAssembled Successfully -- type ppm to view program memory"<<endl;
    
    Core *SP = new Core("SNODE_MC","sp_rfile",s_asm.TISRA, s_asm.RISRA);
    Core *P0 = new Core("PNODE_MC","pn0_rfile",p_asm.TISRA, p_asm.RISRA);
    Core *P1 = new Core("PNODE_MC","pn1_rfile",p_asm.TISRA, p_asm.RISRA);
    Core *P2 = new Core("PNODE_MC","pn2_rfile",p_asm.TISRA, p_asm.RISRA);
    Core *P3 = new Core("PNODE_MC","pn3_rfile",p_asm.TISRA, p_asm.RISRA);
    CommLink SPP0(SP,P0,0x04,0x00,0x09,0x05,0x0A,0x0B);
    
    string command;
    cout<<"Welcome to HCPU -- type help or ? for a command listing"<<endl;
    setDebug(*SP,true);
    setDebug(*P0,false);
    setDebug(*P1,false);
    setDebug(*P2,false);
    setDebug(*P3,false);
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
         if(command == "q")
         {
             exit(0);
         }
         if(command == "ppm")
         {
              SP->Core::printProgramMemory();
         }
         else if(command == "pstage")
         {
            int tempinput;
            cin>>tempinput;
            SP->Pipeline[tempinput-1].print();
         }
         stop = SP->clockCore();
         SPP0.communicate();
    }
    cout<<"BREAK"<<endl;
    setDebug(*SP, false);
    setDebug(*P0, true);
    while(command != "q")
    {
         cout<<"$ ";
         cin>>command;
         if(command == "n")
         {
              
         }
         if(command == "ppm")
         {
              P0->Core::printProgramMemory();
         }
         P0->clockCore();
         SPP0.communicate();
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
