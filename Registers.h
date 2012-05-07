/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Register File for HCPU
*/
#ifndef REGISTERS_H
#define REGISTERS_H
#include <iostream>
#include <fstream>
using namespace std;


class Registers
{
      public:
             unsigned char R0,R1,R2,R3;
             int R0_in_use, R1_in_use, R2_in_use, R3_in_use;
             string RF;
             Registers(string);
             Registers();
             void writeToDisk();
             void readFromDisk();
             void setRegister(int,char);
             void print();
             void resetRegisters();
             char getRegister(int);
};
#endif
