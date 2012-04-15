/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Pipeline stage for HCPU
*/
#ifndef STAGE_H
#define STAGE_H
#include <iostream>
using namespace std;

class Stage
{
      public:
             unsigned char opcode;
             unsigned char operand1;
             unsigned char operand2;
             unsigned char data_in1;
             unsigned char data_in2;
             unsigned char result1;
             unsigned char result2;
             unsigned int  state;
             int speculative;
             int reg1,reg2;
             int stalled;
             int number;
             int cyclesRemaining;
             Stage(int);
             void print();
             void reset();
      private:
};
#endif
