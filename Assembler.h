/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  Assembler for HCPU
*/

#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
using namespace std;


class Assembler
{
      public:
              bool ASSEMBLER_DEBUG;
              int written_count;
              int num_of_operands, is_label,linenumber;
              string opcode, operand1, operand2, operand3;
              int endofprog;
              unsigned char iw1, iw2;
              map<string,char> instructionTable;
              map<string,int> labelStart;
              map<int, string> labelOperand; // keeps track of where a label was used as an operand
                                             // in the case of a label operand, we write blank spots
                                             // will need to go back and fill in the blanks
              char programMemory[256];
              Assembler(string); //Constructor takes opcode filename as input.. 
              ~Assembler();
              void Assemble(string, string);  //Reads in file given in first param, and writes out machine code file to second param
              void parseInstruction(string);  //Given an instruction line, sets opcode, operand1, operand2, and operand3 globals
              void createInstructionWords();  //Based on opcode, operand1, operand2, operand3 sets iw1, iw2
              void outputInstruction();       //Sets programMemory array values to iw1 and iw2
              void clearOutputFile(string);   //Resets/creates machine code file
              char registerNumber(string);    //Returns 0x00,0x01,0x02,0x03 given R0, R1, R2, R3
              void fillInLabelAddresses();    //Runs after all assembly file processing has been done and replaces labels with starting addresses for those labels
              string & rtrim(string&); //rear trim -- see function for credit (online)
              string & ltrim(string &s); //leading trim (front) -- see function for credit (online)
              void writeOutProgramMemory(string);  //Writes program memory array to disk
              void reset(); //Resets appropriate global vars between line iterations
              void print(); //Debug -- prints current line info gathered
              void printInstructionTable(); //Debug -- prints instruction table loaded
              void printError(string,int); //Prints error and line number then quits
      private:
              char convertInstruction(string); //Returns an opcode given an instruction string
};
#endif
