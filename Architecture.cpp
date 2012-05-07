#include <iostream>
#include <sstream>
#include <deque>
#include "Assembler.h"
#include "Registers.h"
#include "Stats.h"
#include "Stage.h"
#include "Core.h"

string main_code = "Assembly/muldivloop.asm";
string machine_code_file = "MACHINE_CODE";

int main(int argc, char * argv[])
{
    Assembler assemble("opcodes"); //Assembler will be implemented later
    assemble.Assemble(main_code, machine_code_file); // Assembler will be implemented later
    cout<<"Assembled Successfully -- type ppm to view program memory"<<endl;
    string command;
    cout<<"Welcome to HCPU -- type help or ? for a command listing"<<endl;
    Core SP = Core(machine_code_file,"sp_rfile");
    int stop = SP.clockCore("n");
    while(stop != 1)
    {
         stop = SP.clockCore("n");
    }
    system("pause");
      
    return 0;
}
