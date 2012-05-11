#include <iostream>
#include <sstream>
#include <deque>
#include <vector>
#include "Assembler.h"
#include "Core.h"
#include "CommLink.h"
using namespace std;

string main_code = "Assembly/muldivloop.asm";
string machine_code_file = "MACHINE_CODE";

bool clockSystem();
int mapProcNameToNum(string);
string mapProcNumToName();
void setDebug(Core*,bool);
void selectCore();
int PCoreInUse[4] = {0,0,0,0};
int selected = 0;
int RUN_FOR = 0;

vector <Core*> cores;
vector <CommLink> comms;

int main(int argc, char * argv[])
{

    Assembler p_asm = Assembler("opcodes"); 
    Assembler s_asm = Assembler("opcodes");
    p_asm.Assemble("Assembly/procnode.asm","PNODE_MC");
    s_asm.Assemble("Assembly/supernode.asm","SNODE_MC");
    
    cout<<"\nAssembled Successfully -- type ppm to view program memory"<<endl;
    
    Core *SP = new Core("SNODE_MC", "sp_rfile",  s_asm.TISRA, s_asm.RISRA);
    Core *P0 = new Core("PNODE_MC", "pn0_rfile", p_asm.TISRA, p_asm.RISRA);
    Core *P1 = new Core("PNODE_MC", "pn1_rfile", p_asm.TISRA, p_asm.RISRA);
    Core *P2 = new Core("PNODE_MC", "pn2_rfile", p_asm.TISRA, p_asm.RISRA);
    Core *P3 = new Core("PNODE_MC", "pn3_rfile", p_asm.TISRA, p_asm.RISRA);
    
    cores.push_back(SP);
    cores.push_back(P0);
    cores.push_back(P1);
    cores.push_back(P2);
    cores.push_back(P3);
    
    selected = 0;
    
    CommLink SPP0("SP","P0",SP,P0,0x04,0x00,0x09,0x05,0x0A,0x0B);
    comms.push_back(SPP0);
    
    string command;
    cout<<"Welcome to HCPU -- type help or ? for a command listing"<<endl;

    int stop;
    int running = 0;
    while(stop != 1)
    {
         selectCore();
         cout<<"Debugging selected for core "<<mapProcNumToName()<<endl;
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
              cores[selected]->printProgramMemory();
              continue;
         }
         if(command == "pdm")
         {
              cores[selected]->printDataMemory();
              continue;
         }
         if(command == "ni")
         {
              cin>>RUN_FOR;
              command = "n";
              running = true;;
         }
         else if(command == "pstage")
         {
            int tempinput;
            cin>>tempinput;
            cores[selected]->Pipeline[tempinput-1].print();
            continue;
         }
         else if(command == "pp")
         {
              cores[selected]->pipePrint();
              continue;
         }
         else if(command == "s")
         {
              string tempinput;
              cin>>tempinput;
              selected = mapProcNameToNum(tempinput);
              continue;
         }
         else if(command == "n")
         {
             stop = clockSystem();
             
             if(RUN_FOR == 0) running = false;
             else RUN_FOR--;
         }
         else
         {
             cout<<"Invalid command!"<<endl;
         }
    }
    system("pause"); 
    return 0;
}
bool clockSystem()
{
    int one = cores[0]->clockCore(); //Clock SP
    comms[0].communicate(); // SP->P0 comms
    int two = cores[1]->clockCore(); //Clock P0
    return one && two;
}
void selectCore()
{
     for(int i = 0; i<cores.size(); i++)
     {
             if(i == selected)
             {
                  setDebug(cores[i], true);
             }
             else
             {
                  setDebug(cores[i], false);
             }
     }
}
int mapProcNameToNum(string name)
{
    if(name == "SP") return 0;
    if(name == "P0") return 1;
    if(name == "P1") return 2;
    if(name == "P2") return 3;
    if(name == "P3") return 4;
}
string mapProcNumToName()
{
   if(selected == 0) return "SP";
   if(selected == 1) return "P0";
   if(selected == 2) return "P1";
   if(selected == 3) return "P2";
   if(selected == 4) return "P3";
}
void setDebug(Core * c, bool debug)
{
    c->PIPE_DEBUG = debug;
    c->PRINT_FINAL_STATE = debug;
    c->PRINT_PC = debug;
    c->CONTROL_DEBUG = debug;
    c->INTERRUPT_DEBUG = debug;
    c->FLAG_DEBUG = debug;
    c->REGISTER_DEBUG = debug;
    c->DEBUG_DFWD = debug;
    c->RW_DEBUG = debug;
}
