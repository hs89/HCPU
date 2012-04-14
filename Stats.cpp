/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Statistics for HCPU
*/

#include "Stats.h"

Stats::Stats()
{
              CPY_CNT = SWAP_CNT = LDD_CNT =
              STD_CNT = LDI_CNT = IN_CNT = OUT_CNT = ADD_CNT =
              SUB_CNT = AND_CNT = CEQ_CNT = CLT_CNT =
              NOT_CNT = MUL_CNT = DIV_CNT = SHLA_CNT = 
              SHLL_CNT = SHRA_CNT = SHRL_CNT = JMP_CNT = BR_CNT =
              CALL_CNT = RET_CNT = RETI_CNT = SIMD_CNT = 0;
              
              I_CNT = 0;
              STALL_CNT = 0;
              IMM_CNT = 0;
              DIS_CNT = 0;
              MC_CNT = 0;
}

void Stats::instructionIssued(unsigned char opcode)
{
    I_CNT++;
    switch(opcode & 0xF0)
    {
        case 0x00:
             //CPY
             CPY_CNT++;
             break;
        case 0x10:
             //SWAP
             SWAP_CNT++;
             break;
        case 0x20:
             //LD or ST
             switch(opcode & 0x02)
             {
                case 0x00:
                      //Immediate
                      IMM_CNT++;
                      switch(opcode & 0x01)
                      {
                          case 0x00:
                               //Load
                               LDI_CNT++;
                               break;
                          case 0x01:
                               //Store
                               //No such command
                               break;
                      }
                      break;
                 case 0x02:
                      //Displacement
                      DIS_CNT++;
                      switch(opcode & 0x01)
                      {
                          case 0x00:
                               //Load
                               LDD_CNT++;
                               break;
                          case 0x01:
                               //Store
                               STD_CNT++;
                               break;
                      }
                      break;
             }
             break;
        case 0x30:
             //IN and OUT
             switch(opcode & 0x01)
             {
                 case 0x00:
                      //Input
                      IN_CNT++;
                      break;
                 case 0x01:
                      //Output
                      OUT_CNT++;
                      break;
             }
             break;
        case 0x40:
             //SHIFT
             switch(opcode & 0x03)
             {
                 case 0x00:
                      //SHLA
                      SHLA_CNT++;
                      break;
                 case 0x01:
                      //SHLL
                      SHLL_CNT++;
                      break;
                 case 0x02:
                      //SHRA
                      SHRA_CNT++;
                      break;
                 case 0x03:
                      //SHRL
                      SHRL_CNT++;
                      break;
             }
             break;
        case 0x50:
             //JMP or BRANCH
             switch(opcode & 0x0F)
             {
                  case 0x00:
                       //JMP Unconditionally
                       JMP_CNT++;
                       break;
                  case 0x01:
                       //BR if V = 1
                       BR_CNT++;
                       break;
                  case 0x02:
                       //BR if Z = 1
                       BR_CNT++;
                       break;
                  case 0x04:
                       //BR if N = 1
                       BR_CNT++;
                       break;
                  case 0x08:
                       //BR if C = 1
                       BR_CNT++;
                       break;
                  case 0x0A:
                       //BR if C&Z = 1
                       BR_CNT++;
                       break;
                  case 0x0C:
                       //BR if C&N = 1
                       BR_CNT++;
                       break;
                  case 0x0F:
                       //CALL
                       CALL_CNT++;
                       break;
             }
             break;
        case 0x60:
             //RET or RETI
             switch(opcode & 0x08)
             {
                 case 0x00:
                      //Interrupt
                      RETI_CNT++;
                      break;
                 case 0x08:
                      //Subroutine
                      RET_CNT++;
                      break;
             }
             break;
        case 0x70:
             //SIMD
             SIMD_CNT++;
             break;
        case 0x80:
             //ADD
             ADD_CNT++;
             break;
        case 0x90:
             //SUB
             SUB_CNT++;
             break;
        case 0xA0:
             //AND
             AND_CNT++;
             break;
        case 0xB0:
             //CEQ
             CEQ_CNT++;
             break;
        case 0xC0:
             //CLT
             CLT_CNT++;
             break;
        case 0xD0:
             //MUL
             MUL_CNT++;
             break;
        case 0xE0:
             //DIV
             DIV_CNT++;
             break;
        case 0xF0:
             //NOT
             NOT_CNT++;
             break;             
    }
}

void Stats::pipelineStall()
{
     STALL_CNT+=1;
}

void Stats::printStuff(string name, int val)
{
     cout<<name<<" = "<<dec<<(int)val<<endl;
}

void Stats::print()
{
     printStuff("COPY COUNT",CPY_CNT);
     printStuff("SWAP COUNT",SWAP_CNT);
     printStuff("LD DISPLACEMENT COUNT",LDD_CNT);
     printStuff("LD IMMEDIATE COUNT",LDI_CNT);
     printStuff("ST DISPLACEMENT COUNT",STD_CNT);
     printStuff("IN COUNT",IN_CNT);
     printStuff("OUT COUNT",OUT_CNT);
     printStuff("ADD COUNT",ADD_CNT);
     printStuff("SUB COUNT",SUB_CNT);
     printStuff("AND COUNT",AND_CNT);
     printStuff("CEQ COUNT",CEQ_CNT);
     printStuff("CLT COUNT",CLT_CNT);
     printStuff("NOT COUNT",NOT_CNT);
     printStuff("MUL COUNT",MUL_CNT);
     printStuff("DIV COUNT",DIV_CNT);
     printStuff("SHLA COUNT",SHLA_CNT);
     printStuff("SHLL COUNT",SHLL_CNT);
     printStuff("SHRA COUNT",SHRA_CNT);
     printStuff("SHRL COUNT",SHRL_CNT);
     printStuff("JMP COUNT",JMP_CNT);
     printStuff("BR COUNT",BR_CNT);
     printStuff("CALL COUNT",CALL_CNT);
     printStuff("RET COUNT",RET_CNT);
     printStuff("RETI COUNT",RETI_CNT);
     printStuff("SIMD COUNT",SIMD_CNT);
     printStuff("INSTRUCTION COUNT",I_CNT);
     printStuff("STALL COUNT",STALL_CNT);
     printStuff("IMMMEDIATES COUNT",IMM_CNT);
     printStuff("DISPLACEMENT COUNT",DIS_CNT);
     printStuff("MC COUNT",MC_CNT);

}


