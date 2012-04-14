/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Statistics for HCPU
*/
#ifndef STATS_H
#define STATS_H
#include <iostream>
using namespace std;
class Stats
{
      public:
             int CPY_CNT, SWAP_CNT, LDD_CNT,
             STD_CNT, LDI_CNT, IN_CNT, OUT_CNT, ADD_CNT,
             SUB_CNT, AND_CNT, CEQ_CNT, CLT_CNT,
             NOT_CNT, MUL_CNT, DIV_CNT, SHLA_CNT, 
             SHLL_CNT, SHRA_CNT, SHRL_CNT, JMP_CNT, BR_CNT,
             CALL_CNT, RET_CNT, RETI_CNT, SIMD_CNT;
             
             int I_CNT;
             int STALL_CNT;
             int IMM_CNT;
             int DIS_CNT;
             int MC_CNT;
             
             Stats();
             void printStuff(string,int);
             void instructionIssued(unsigned char);
             void pipelineStall();
             void print();

};
#endif
