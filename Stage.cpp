/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Pipeline stage for HCPU
*/
#include "Stage.h"

Stage::Stage(int num)
{
     number = num;
     opcode = 0x00;
     operand1 = 0x00;
     operand2 = 0x00;
     result1 = 0x00;
     result2 = 0x00;
     data_in1 = 0x00;
     data_in2 = 0x00;
     state = 0;
     cyclesRemaining = -1;
     stalled = 0;
     reg1=reg2=-1;
     hasop1 = hasop2 = false;
     dfwd = 0;
     speculate = 0;
}

void Stage::print()
{
    if(cyclesRemaining != -1)
    {
        cout<<"Stage number = "<<number<<endl;
        cout<<"Opcode = "<<(int)opcode<<endl;
        cout<<"State = "<<state<<endl;
        cout<<"Cycles Left = "<<cyclesRemaining<<endl;
        cout<<"Operand 1 = "<<(int)operand1<<endl;
        cout<<"Operand 2 = "<<(int)operand2<<endl;
        printf("Data_in1 = %02X\n",data_in1);
        printf("Data_in2 = %02X\n",data_in2);
        cout<<"Result 1 = "<<(int)result1<<endl;
        cout<<"Result 2 = "<<(int)result2<<endl;
        cout<<"Reg1 = "<<reg1<<endl;
        cout<<"Reg2 = "<<reg2<<endl;
    }
}

void Stage::reset()
{
     opcode = 0x00;
     operand1 = 0x00;
     operand2 = 0x00;
     result1 = 0x00;
     result2 = 0x00;
     data_in1 = 0x00;
     data_in2 = 0x00;
     state = 0;
     cyclesRemaining = reg1 = reg2 = -1;
     speculate = 0;
     hasop1 = hasop2 = false;
     dfwd = 0;
}
