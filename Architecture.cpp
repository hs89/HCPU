/*
  Holden Sandlar
  Advanced Computer Architecture
  0301-810 w/ Dorin Patru
  PIPELINED MIPS PROCESSOR SIMULATOR
*/

#include <iostream>
#include <sstream>
#include <deque>
#include "Assembler.h"
#include "Registers.h"
#include "Stats.h"
#include "Stage.h"

using namespace std;

/*    DEBUG FLAG DEFS       */
bool PIPE_DEBUG = true;
bool PRINT_FINAL_STATE = true;
bool PRINT_PC = true;
bool CONTROL_DEBUG = true;
bool INTERRUPT_DEBUG = true;
bool FLAG_DEBUG = true;
bool REGISTER_DEBUG = true;
/*    END DEBUG FLAG DEFS    */

/*  Files to be loaded:  */
string register_file = "registerfile";
string opcode_file = "opcodes";
string cycles_file = "cyclesperinstruction";
string code_file = "code.asm";
string machine_code_file = "MACHINE_CODE";
/*  END FILE DEFS        */

unsigned char PM[256]; //Program Memory Space
unsigned char DM[256]; //Data Memory Space
unsigned char IO[256]; //IO Memory Space
unsigned char PC = 0x00; // Program Memory Counter
unsigned char DC = 0x00; // Data Memory Counter
unsigned char SP = 0xFF; // Stack Pointer
unsigned int Cycles[16]; //Cycles per instruction (index is opcode)

deque <int> StagesExecuting;

/*    GLOBAL FLAG DEFS      */
bool PIPE_STALLED = false;
int STOP_FILLING_PIPELINE = 0;
int PC_STALLED = 0;
int INTERRUPTED = 0;
bool PIPE_FULL = false;
bool END_REACHED = false;
int Z_FLAG, N_FLAG, C_FLAG, V_FLAG;
int RUNNING = 0;
int RUN_FOR = 0;
int PMEND;
int instructionClocked = 0;
/*     END GLOBAL FLAG DEFS   */

/* FUNCTION PROTOTYPE DEFS */
void testRegisters(); //Prototype for register class test -- debug code
void printState(); //Debug code
void loadCyclesPerInstruction(string); //Prototype to initialize the Cycles array
void printCyclesPerInstruction(); //Debug code
void loadProgramMemory(string); //Loads program memory from a file
void printProgramMemory(); //Prints program memory 
void printDataMemory(); //Prints data memory
void pipePrint(); //Prints the state of the pipeline
void clockPipeline(); //Clocks a machine cycle of the pipeline
void clockStage(Stage&); //Clocks a machine cycle of a given stage
void DOF(Stage&); //Data operand fetch
void iF(Stage&); // Instruction Fetch
void execute(Stage&); //Execution
unsigned char execAdd(unsigned char,unsigned char); //Do ADD Instruction on given operands
void MWB(Stage&); //Memory Writeback
void WB(Stage&); //Register Writeback
void resetFlags(); //Reset C,V,Z,N
void printFlags(); //Print Flags
void saveSystemState(); //Saves system state to the stack
void restoreSystemState(); //Restores system state from the stack
bool checkDependence(Stage&); //Checks if opcode of a stage is dependent on a previous operation
bool allStagesComplete(); //Returns whether or not all pipeline stages are complete
void finishPipelineExecution(); //Services all instructions in pipeline until pipeline empty
int findStageInQueue(int); //Finds the index of a stage in the instruction issued queue
bool determineIfDependent(int, char); //Called by checkDependence, actually does the register dependence checking
void resetAnyFinishedStages(); //Checks if a pipeline stage needs to be reset
void resetStage(Stage&); //Resets a pipeline stage
int stallStageUntilAllOtherStagesFinished(int);  //Returns stall value -- used in accounting when trying to stall a stage until all other stages have completed
void registerInterrupts(); //Checks the interrupt vectors and calls ISR if necessary
void serviceInterrupt(char, int); //Services an interrupt
string stateNumToString(Stage); //Converts a stage's state number to a string (iF, DOF, WB, etc)
bool anyStageStalled(); //checks if any stage in the pipeline is stalled
bool pipelineFull(); //checks if the pipeline is full of instructions
void printStageQueue(); //For debug.. prints the pipeline stage queue (used to determine data dependence)
bool checkStageRegisterDependence(int,int); //Checks to see if a register number is in use by given stage number (for determining data dependence)
/* END FUNCTION PROTOTYPE DEFS */

/* GLOBAL VAR DEFS */
Registers RF(register_file); //Create a register file
Stats Statistics; //Create a statistics object
Stage s1(1), s2(2), s3(3), s4(4), s5(5); //Create pipeline stages
/* END GLOBAL VAR DEFS */

int main(int argc, char * argv[])
{
   if(argc > 1 && argc < 3)
    {
            //user entered a filename in the command line
            //use this filename as the code file
            code_file = argv[1];
            cout<<"Code file set to "<<code_file<<endl;
    }
    
    Assembler assemble(opcode_file); //Assembler will be implemented later
    assemble.Assemble(code_file, machine_code_file); // Assembler will be implemented later
    cout<<"Assembled Successfully -- type ppm to view program memory"<<endl;
    
    loadCyclesPerInstruction(cycles_file);    
    loadProgramMemory(machine_code_file);  
    resetFlags();
    
    //printProgramMemory();
    //printCyclesPerInstruction(); //Prints cycles per instruction for each instruction
    //testRegisters(); //Tests the Register File Class   
    
    string command;
    cout<<"Welcome to HCPU -- type help or ? for a command listing"<<endl;
    for(;;)
    {
           if(RUNNING == 0 && RUN_FOR == 0)
           {
                      cout<<"$ ";
                      cin>>command;
           }
           if(RUN_FOR != 0)
           {
                      command = "n";
                      RUN_FOR--;
           }
           if(PC == PMEND && allStagesComplete())
           {
                 if(PRINT_FINAL_STATE)
                 {
                     Statistics.print();
                     RF.print();
                     printFlags();
                     PRINT_FINAL_STATE = false;
                 }
                 RUNNING = 0;
           }
           if(command == "exit" || command == "q") exit(0);
           else if(command == "info") { printState(); continue; }
           else if(command == "help" || command == "?")
           {
                cout<<
                "-----------------\n"<<
                "Control Commands \n"<<
                "-----------------\n"<<
                "(exit || q) - quit simulator\n"<<
                "(help || ?) - show this help menu\n"<<
                "n           - clock next machine cycle\n"<<
                "ni <number> - clock <number> cycles\n"<<
                "c           - clock machine cycles automatically to end of program\n"<<
                "            \n"<<
                "-----------------\n"<<
                "Debug Commands   \n"<<
                "-----------------\n"<<
                "pdf - PRINT TOGGLE FLAGS STATUS'\n"<<
                "ppc - Print PC Debug TOGGLE\n"<<
                "pd -  Pipe Debug TOGGLE\n"<<
                "pfc - Print final statistics TOGGLE\n"<<
                "pcd - Print control debug TOGGLE\n"<<
                "pid - Print interrupt debug TOGGLE\n"<<
                "pfd - Print flag debug TOGGLE\n"<<
                "prd - Print register debug TOGGLE\n"<<
                "            \n"<<
                "-----------------\n"<<
                "Info Commands    \n"<<
                "-----------------\n"<<
                "pp  - Print Pipeline info\n"<<
                "ps  - Print statistics\n"<<
                "pr  - Print registers\n"<<
                "pf  - Print Flags\n"<<
                "pi  - Print general info\n"<<
                "ppm - Print Program Memory\n"<<
                "pdm - Print Data Memory\n"<<endl;
                continue;
           }
           else if(command == "pp")
           {
                //Print pipeline state
                pipePrint();
                continue;
           }
           else if(command == "pdf")
           {
                //Print toggle flag status's
                cout<<"Toggle flags status:\n"<<
                "PRINT_PC = "<<PRINT_PC<<endl<<
                "PRINT_FINAL_STATE = "<<PRINT_FINAL_STATE<<endl<<
                "CONTROL_DEBUG = "<<CONTROL_DEBUG<<endl<<
                "INTERRUPT_DEBUG = "<<INTERRUPT_DEBUG<<endl<<
                "FLAG_DEBUG = "<<FLAG_DEBUG<<endl<<
                "REGISTER_DEBUG = "<<REGISTER_DEBUG<<endl<<
                "PIPEINE_DEBUG = "<<PIPE_DEBUG<<endl;
                continue;
           }
           else if(command == "ppc")
           {
                if(PRINT_PC == 1) { PRINT_PC = 0; cout<<"PC debug turned off"<<endl; continue; }
                if(PRINT_PC == 0) { PRINT_PC = 1; cout<<"PC debug turned on"<<endl; continue; }
           }
           else if(command == "pfc")
           {
                if(PRINT_FINAL_STATE == 1) { PRINT_FINAL_STATE = 0; cout<<"Final state print turned off"<<endl; continue; }
                if(PRINT_FINAL_STATE == 0) { PRINT_FINAL_STATE = 1; cout<<"Final state print turned on"<<endl; }
           }
           else if(command == "pcd")
           {
                if(CONTROL_DEBUG == 1) { CONTROL_DEBUG = 0; cout<<"Control debug turned off"<<endl; continue; }
                if(CONTROL_DEBUG == 0) { CONTROL_DEBUG = 1; cout<<"Control debug turned on"<<endl; continue; }
           }
           else if(command == "pid")
           {
                if(INTERRUPT_DEBUG == 1) { INTERRUPT_DEBUG = 0; cout<<"Interrupt debug turned off"<<endl; continue; }
                if(INTERRUPT_DEBUG == 0) { INTERRUPT_DEBUG = 1; cout<<"Interrupt debug turned on"<<endl; continue; }
           }
           else if(command == "pfd")
           {
                if(FLAG_DEBUG == 1) { FLAG_DEBUG = 0; cout<<"Flag debug turned off"<<endl; continue; }
                if(FLAG_DEBUG == 0) { FLAG_DEBUG = 1; cout<<"Flag debug turned on"<<endl; continue; }
           }
           else if(command == "prd")
           {
                if(REGISTER_DEBUG == 1) { REGISTER_DEBUG = 0; cout<<"Register debug turned off"<<endl; continue; }
                if(REGISTER_DEBUG == 0) { REGISTER_DEBUG = 1; cout<<"Register debug turned on"<<endl; continue; }
           }
           else if(command == "ppm")
           {
                printProgramMemory();
                continue;
           }
           else if(command == "pd")
           {
                if(PIPE_DEBUG == 1) { PIPE_DEBUG = 0; cout<<"Pipe debug turned off"<<endl; continue; }
                if(PIPE_DEBUG == 0) { PIPE_DEBUG = 1; cout<<"Pipe debug turned on"<<endl; continue; }
           }
           else if(command == "pdm")
           {
                printDataMemory();
                continue;
           }
           else if(command == "pf")
           {
                //Print current flag values
                printFlags();
                continue;
           }
           else if(command == "c")
           {
                //Run to end of program
                command = "n";
                RUNNING = 1;
                continue;
           }
           else if(command == "ps")
           {
                //Print statistics
                Statistics.print();
                continue;
           }
           else if(command == "pr")
           {
                //Print register file
                RF.print();
                continue;
           }
           else if(command == "ni")
           {
                cout<<"How many machine cycles would you like to clock? : ";
                cin>>RUN_FOR;
                continue;
                                
           }
           else if(command == "n")
           {
                if(PC == PMEND && !allStagesComplete())
                {
                     cout<<"************************** PROGRAM END REACHED **************************"<<endl;
                     finishPipelineExecution();
                     continue;
                }
                else if(PC == PMEND && allStagesComplete())
                {
                     cout<<"************************** PROGRAM END REACHED **************************"<<endl;
                     continue;
                }
                
                if(PIPE_DEBUG) cout<<"------------- MC "<<dec<<(int)Statistics.MC_CNT<<" -------------"<<endl;
                clockPipeline();
                
                if(PC_STALLED != 1 && STOP_FILLING_PIPELINE != 1 && !PIPE_FULL || INTERRUPTED == 1)
                {
                      PC++;
                }
                registerInterrupts();
                continue;
           }
           else
           {
               cout<<"Invalid command"<<endl;
               continue;
           }
    }    
    return 0;
}

void clockPipeline()
{     
     if(PC == PMEND) END_REACHED = true;
     if(PRINT_PC) cout<<"PC = "<<hex<<(int)PC<<endl;
     if(STOP_FILLING_PIPELINE == 1) instructionClocked = 1;
     else instructionClocked = 0;
     PIPE_FULL = pipelineFull();
     if(PIPE_FULL) instructionClocked = 1;

     Statistics.MC_CNT++;
     
     clockStage(s1);
     if(STOP_FILLING_PIPELINE == 1) instructionClocked = 1;
     resetAnyFinishedStages(); 
     
     clockStage(s2);
     if(STOP_FILLING_PIPELINE == 1) instructionClocked = 1;
     resetAnyFinishedStages(); 
     
     clockStage(s3);
     if(STOP_FILLING_PIPELINE == 1) instructionClocked = 1;
     resetAnyFinishedStages(); 
     
     clockStage(s4);
     if(STOP_FILLING_PIPELINE == 1) instructionClocked = 1;
     resetAnyFinishedStages(); 
     
     clockStage(s5);
     instructionClocked = 0; 
     resetAnyFinishedStages();
     if(anyStageStalled()) Statistics.STALL_CNT++;
      
     if(PIPE_DEBUG) pipePrint();
     if(REGISTER_DEBUG) RF.print();  
}

void clockStage(Stage & stagenum)
{
     switch(stagenum.state)
     {
         case 0:
              //Nothing in stage yet -- do iF/Decode
              if((!END_REACHED && instructionClocked != 1) || (INTERRUPTED == 1 && instructionClocked != 1))
              {
                  iF(stagenum);   
                  StagesExecuting.push_front(stagenum.number);           
                  stagenum.state += 2;
                  stagenum.cyclesRemaining -= 2;
                  instructionClocked = 1;
              }
              break;
         case 1:
              //Stage in instructionFetch/Decode
              cout<<"In the clockStage switch for instruction Fetch ... how?"<<endl;
              break;
         case 2:
              //Stage in Data Operand Fetch
              DOF(stagenum);
              if(stagenum.stalled != 1)
              {
                  stagenum.state++;
                  stagenum.cyclesRemaining--;
              }
              else
              {
                  return;
              }
              break;
         case 3:
              //Stage in execution
             execute(stagenum);
             if(stagenum.cyclesRemaining > 2)
             {
                  stagenum.cyclesRemaining--;
             }
             else
             {
                  stagenum.cyclesRemaining--;
                  stagenum.state++;
             }
             break;
         case 4:
              //Stage in Memory Writeback
              MWB(stagenum);
              stagenum.cyclesRemaining--;
              stagenum.state++;
              break;
         case 5:
              //Stage in Register Writeback
              WB(stagenum);
              stagenum.state++;
              break;
     }
}

void iF(Stage & stagenum)
{
      stagenum.opcode = (PM[PC] & 0xF0)>>4;
      stagenum.operand1 = (PM[PC] & 0x0F);
      stagenum.operand2 = 0x00;
      stagenum.cyclesRemaining = Cycles[stagenum.opcode];
      Statistics.instructionIssued(PM[PC]);
      
      if(stagenum.opcode == 0x02 || //LD/ST
         stagenum.opcode == 0x03 || //I/O
         stagenum.opcode == 0x05 || //JMP/BR
         stagenum.opcode == 0x07  ) //SIMD
      {
         //Will need another instruction word
         PC++;
         stagenum.operand2 = (PM[PC] & 0xFF);
      }
      
      //To do the accounting of pipeline stalls
      //mark which registers will be in use right now
      switch((int)stagenum.opcode)
      {
             case 0x00:
                  //CPY
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  break;
             case 0x01:
                  //SWAP
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  stagenum.reg2 = (int)((stagenum.operand1&0x03));
                  break;
             case 0x02:
                  //LD/ST
                  if((stagenum.opcode & 0x01) == 0x00)
                  {
                      //Load into a register -- mark destination register
                      stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  }
                  break;
             case 0x03:
                  //IN/OUT
                  if(stagenum.opcode & 0x01 == 0x00)
                  {
                      //IN -- mark destination register
                      stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  }
                  STOP_FILLING_PIPELINE = 1;
                  PC_STALLED = 1;
                  break;
             case 0x04:
                  //SHIFT
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  break;
             case 0x05:
                  //JMP/BR/CALL
                  //In the case of Jump, branch, or call we want to stop
                  //filling the pipeline to ensure we don't execute something in error
                  //also stall the program counter because we're going to change it..
                  //don't want to increment it in error
                  STOP_FILLING_PIPELINE = 1;
                  PC_STALLED = 1;
                  break;
             case 0x06:
                  //RET/RETI
                  STOP_FILLING_PIPELINE = 1;
                  PC_STALLED = 1;
                  break;
             case 0x07:
                  //SIMD
                  //TODO: Implement Later
                  break;
             case 0x08:
                  //ADD
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  break;
             case 0x09:
                  //SUB
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  break;
             case 0x0A:
                  //AND
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  break;
             case 0x0B:
                  //CEQ
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  break;
             case 0x0C:
                  //CLT
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  break;
             case 0x0D:
                  //MUL
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  stagenum.reg2 = (int)((stagenum.operand2&0x03));
                  break;
             case 0x0E:
                  //DIV
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  stagenum.reg2 = (int)((stagenum.operand2&0x03));
                  break;
             case 0x0F:
                  //NOT
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  break;
      }
}

void DOF(Stage & stagenum)
{
     bool dependent = checkDependence(stagenum);
     if(dependent) {
          stagenum.stalled = 1;
     }
     else { 
          stagenum.stalled = 0;
     }
     if(stagenum.stalled == 1)
     {
         return;
     }
     else
     {
         switch((int)stagenum.opcode)
         {
             case 0x00:
                  //CPY
                  stagenum.data_in1 = RF.getRegister(stagenum.opcode & 0x03);
                  break;
             case 0x01:
                  //SWAP -- this op should really take 2 MC's to execute
                  stagenum.data_in1 = RF.getRegister((stagenum.opcode & 0x0C) >> 2);
                  stagenum.data_in2 = RF.getRegister( stagenum.opcode & 0x03);
                  break;
             case 0x02:
                  //LD/ST
                  //Data in for a Load/Store is the value in Ry, or the displacement value
                  switch(stagenum.operand1 & 0x03)
                  {
                      case 0x00:
                           //Load Immediate
                           stagenum.data_in1 = stagenum.operand2;
                           break;
                      case 0x01:
                           //Store Immediate
                           //No such thing
                           break;
                      case 0x02:
                           //Load Displacement
                           stagenum.data_in1 = RF.getRegister((stagenum.operand2 & 0xC0)>>6);
                           stagenum.data_in2 = stagenum.operand2 & 0x3F;
                           break;
                      case 0x03:
                           //Store Displacement
                           stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                           stagenum.data_in2 = RF.getRegister((stagenum.operand2 & 0xC0)>>6);
                           break;
                  }
                  break;
             case 0x03:
                  //IN/OUT
                  //If we are pushing a value out to IO, get that data from register
                  switch(stagenum.operand1 & 0x01)
                  {
                      case 0x00:
                           //Input
                           
                           break;
                      case 0x01:
                           //Output
                           stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                           //TODO: Come back and fix this -- this is a workaround because the stage doesn't stall properly
                           
                           break;
                  }
                  stagenum.stalled = stallStageUntilAllOtherStagesFinished(stagenum.number);
                  break;
             case 0x04:
                  //SHIFT
                  stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  break;
             case 0x05:
                  {
                      //JMP/BR
                      stagenum.data_in1 = stagenum.operand2;
                      //we want to stall this stage now until all other stages have finished executing
                      stagenum.stalled = stallStageUntilAllOtherStagesFinished(stagenum.number);
                      return;
                  }
                  break;
             case 0x06:
                  //RET/RETI
                  switch(stagenum.operand1 & 0x04)
                  {
                      case 0x00:
                           //No return value to fetch
                           stagenum.stalled = stallStageUntilAllOtherStagesFinished(stagenum.number);
                           break;
                      case 0x04:
                           //Return value held in register
                           stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x03));
                           stagenum.stalled = stallStageUntilAllOtherStagesFinished(stagenum.number);
                           break;
                  }
                  break;
             case 0x07:
                  //SIMD
                  //TODO: Implement Later
                  break;
             case 0x08:
                  //ADD
                  stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  stagenum.data_in2 = RF.getRegister(stagenum.operand1 & 0x03);
                  break;
             case 0x09:
                  //SUB
                  stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  stagenum.data_in2 = RF.getRegister(stagenum.operand1 & 0x03);
                  break;
             case 0x0A:
                  //AND
                  stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  stagenum.data_in2 = RF.getRegister(stagenum.operand1 & 0x03);
                  break;
             case 0x0B:
                  //CEQ
                  stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  stagenum.data_in2 = RF.getRegister(stagenum.operand1 & 0x03);
                  break;
             case 0x0C:
                  //CLT
                  stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  stagenum.data_in2 = RF.getRegister(stagenum.operand1 & 0x03);
                  break;
             case 0x0D:
                  //MUL
                  stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  stagenum.data_in2 = RF.getRegister(stagenum.operand1 & 0x03);
                  break;
             case 0x0E:
                  //DIV
                  stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  stagenum.data_in2 = RF.getRegister(stagenum.operand1 & 0x03);
                  break;
             case 0x0F:
                  //NOT
                  stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  break;
         }
     }
}
void execute(Stage & stagenum)
{
     switch((int)stagenum.opcode)
     {
         case 0x00:
              //CPY
              {
                   stagenum.result1 = stagenum.data_in1;
              }
              break;
         case 0x01:
              //SWAP -- this op should really take 2 MC's to execute
              {
                  char temp = stagenum.data_in1;
                  stagenum.result1 = stagenum.data_in2;
                  stagenum.result2 = temp;
              }
              break;
         case 0x02:
              //LD/ST
              switch(stagenum.operand1 & 0x03)
              {
                  case 0x00:
                       //Load Immediate
                       stagenum.result1 = stagenum.data_in1;
                       break;
                  case 0x01:
                       //Store Immediate
                       //No such command
                       break;
                  case 0x02:
                       //Load Displacement
                       stagenum.result1 = stagenum.data_in1 + stagenum.data_in2; //Computing MAeff
                       break;
                  case 0x03:
                       //Store Displacement
                       stagenum.result1 = stagenum.data_in1; // This is the value we will store
                       stagenum.result2 = stagenum.data_in2 + (stagenum.operand2 & 0x3F); //This is the location we will store to
                       break;
              }
              break;
         case 0x03:
              //IN/OUT
              switch(stagenum.operand1 & 0x01)
              {
                  case 0x00:
                       //Input
                       break;
                  case 0x01:
                       //Output
                       stagenum.result1 = stagenum.data_in1;
                       break;
              }
              break;
         case 0x04:
              //SHIFT
              switch(stagenum.operand1 & 0x03)
              {
                  case 0x00: case 0x01:
                       {
                           //SHLA, SHLL
                            stagenum.result1 = stagenum.data_in1<<1;
                       }
                       break;
                  case 0x02:
                       {
                           //SHRA
                           char temp = stagenum.data_in1 & 0x80;
                           stagenum.result1 = stagenum.data_in1>>1 | temp;
                       }
                       break;
                  case 0x03:
                       {
                           //SHRL
                           stagenum.result1 = stagenum.data_in1>>1 & 0x7F;
                       }
                       break;
              }
              break;
         case 0x05:
              //JMP/BRANCH
              stagenum.result1 = stagenum.data_in1;
              break;
         case 0x06:
              //RET/RETI
              switch(stagenum.operand1 & 0x04)
              {
                  case 0x00:
                       //No return value to fetch
                       break;
                  case 0x04:
                       //Return value held in register
                       stagenum.result1 = stagenum.data_in1;
                       break;
              }
              break;
         case 0x07:
              //SIMD
              
              break;
         case 0x08:
              //ADD
              {
                  stagenum.result1 = execAdd(stagenum.data_in1,stagenum.data_in2);
              }
              break;
         case 0x09:
              //SUB
              {
                  stagenum.result1 = (int)stagenum.data_in1 - (int)stagenum.data_in2;    
              }
              break;
         case 0x0A:
              //AND
              {
                  stagenum.result1 = (int)stagenum.data_in1 & (int)stagenum.data_in2; 
              }
              break;
         case 0x0B:
              //CEQ
              {
                   if((int)stagenum.data_in1 == (int)stagenum.data_in2)
                   {
                       Z_FLAG = 1;
                   }
                   else
                   {
                       Z_FLAG = 0;
                   }
                   if(FLAG_DEBUG) cout<<"Set Z_FLAG to "<<(int)Z_FLAG<<endl;
              }
              break;
         case 0x0C:
              //CLT
              {
                   if((int)stagenum.data_in1 < (int)stagenum.data_in2)
                   {
                       N_FLAG = 1;
                   }
                   else
                   {
                       N_FLAG = 0;
                   }
                   if(FLAG_DEBUG) cout<<"Set N_FLAG to "<<(int)N_FLAG<<endl;
              }
              break;
         case 0x0D:
              //MUL
              stagenum.result1 = (stagenum.data_in1 * stagenum.data_in2)>>8;
              stagenum.result2 = (stagenum.data_in1 * stagenum.data_in2);
              break;
         case 0x0E:
              //DIV
              stagenum.result1 = (stagenum.data_in1 / stagenum.data_in2);
              stagenum.result2 = (stagenum.data_in1 % stagenum.data_in2);
              break;
         case 0x0F:
              //NOT
              stagenum.result1 = ~stagenum.data_in1;
              break;
     }
}

void MWB(Stage & stagenum)
{
      switch((int)stagenum.opcode)
      {
         case 0x02:
              //LD/ST
              switch(stagenum.operand1 & 0x03)
              {
                  case 0x00:
                       //Load Immediate
                       break;
                  case 0x01:
                       //Store Immediate
                       //No such command
                       break;
                  case 0x02:
                       //Load Displacement
                       break;
                  case 0x03:
                       //Store Displacement
                       DM[stagenum.result2] = stagenum.result1;
                       printf("Stored %02X to DM[%02X]",stagenum.result1,stagenum.result2);
                       break;
              }
              break;
         case 0x03:
              //IN/OUT
              switch(stagenum.operand1 & 0x01)
              {
                  case 0x00:
                       //Input
                       stagenum.result1 = IO[stagenum.operand2];
                       break;
                  case 0x01:
                       //Output
                       IO[stagenum.operand2] = stagenum.result1;
                       printf("Wrote %02X to IO[%02X]",stagenum.result1,stagenum.operand2);
                       break;
              }
              break;
         case 0x05:
              //JMP/BRANCH
              
              break;
         case 0x06:
              //RET/RETI
              
              break;
         case 0x07:
              //SIMD
              
              break;
         case 0x0D:
              //MUL
              
              break;
         case 0x0E:
              //DIV
              //TODO: Implement Later
              break;
         case 0x0F:
              //NOT
              break;
      }
}

void WB(Stage & stagenum)
{
      switch((int)stagenum.opcode)
      {
         case 0x00:
              //CPY
              {
                   RF.setRegister((int)(stagenum.operand1 & 0x0C)>>2, stagenum.result1);
              }
              break;
         case 0x01:
              //SWAP -- this op should really take 2 MC's to execute
              {
                  RF.setRegister((int)(stagenum.operand1 & 0x0C)>>2, stagenum.result2);
                  RF.setRegister((int)(stagenum.operand1 & 0x03)   , stagenum.result1);
              }
              break;
         case 0x02:
              //LD/ST
              switch(stagenum.operand1 & 0x03)
              {
                  case 0x00:
                       //Load Immediate
                       RF.setRegister((stagenum.operand1 & 0x0C)>>2, stagenum.result1);
                       break;
                  case 0x01:
                       //Store Immediate
                       break;
                  case 0x02:
                       //Load Displacement
                       printf("Loaded DM[%02X] = %02X",stagenum.result1,DM[stagenum.result1]);
                       RF.setRegister((stagenum.operand1 & 0x0C)>>2, DM[stagenum.result1]);
                       break;
                  case 0x03:
                       //Store Displacement
                       break;
              }
              break;
         case 0x03:
              //IN/OUT
              switch(stagenum.operand1 & 0x01)
              {
                  case 0x00:
                       //Input
                       RF.setRegister((stagenum.operand1 & 0x0C)>>2, stagenum.result1);
                       printf("Read %02X from IO[%02X]",RF.getRegister((stagenum.operand1&0x0C)>>2),stagenum.operand2);
                       break;
                  case 0x01:
                       //Output
                       break;
              }    
              STOP_FILLING_PIPELINE = 0;
              PC_STALLED = 0;                            
              break;
         case 0x04:
              //SHIFT
              RF.setRegister((stagenum.operand1 & 0x0C) >> 2, stagenum.result1);
              break;
         case 0x05:
              //JMP/BRANCH
              switch(stagenum.operand1 & 0x0F)
              {
                  case 0x00:
                       //JMP Unconditionally
                       PC = (char)((int)stagenum.result1-1); // The -1 accounts for the PC being incremented after completeion of this stage
                       break;
                  case 0x01:
                       //BR if V = 1
                       if(V_FLAG == 1) PC = (char)((int)stagenum.result1-1);
                       break;
                  case 0x02:
                       //BR if Z = 1
                       if(Z_FLAG == 1) PC = (char)((int)stagenum.result1-1);
                       break;
                  case 0x04:
                       //BR if N = 1
                       if(N_FLAG == 1) PC = (char)((int)stagenum.result1-1);
                       break;
                  case 0x08:
                       //BR if C = 1
                       if(C_FLAG == 1) PC = (char)((int)stagenum.result1-1);
                       break;
                  case 0x0A:
                       //BR if C&Z = 1
                       if(C_FLAG == 1 && Z_FLAG == 1) PC = (char)((int)stagenum.result1-1);
                       break;
                  case 0x0C:
                       //BR if C&N = 1
                       if(C_FLAG == 1 && N_FLAG == 1) PC = (char)((int)stagenum.result1-1);
                       break;
                  case 0x0F:
                       //CALL
                       if(CONTROL_DEBUG)
                       {
                           cout<<"Calling a subroutine. Stored system state: "<<endl;
                           RF.print();
                           cout<<"PC Stored = "<<hex<<(int)PC<<endl;
                       }
                       saveSystemState();
                       PC = (char)((int)stagenum.result1-1);
                       RF.resetRegisters();
                       break; 
              }
              STOP_FILLING_PIPELINE = 0; // Re-enable filling of the pipeline now that we have written back the updated PC
              PC_STALLED = 0;
              PIPE_FULL = false;
              PIPE_STALLED = false;
              break;
         case 0x06:
              //RET/RETI
              restoreSystemState();
              if(CONTROL_DEBUG)
              {
                  cout<<"Restoring system state"<<endl;
                  RF.print();
                  cout<<"PC Restored = "<<hex<<(int)PC<<endl;
              }
              STOP_FILLING_PIPELINE = 0;
              PC_STALLED = 0;
              break;
         case 0x07:
              //SIMD
              break;
         case 0x08: //ADD
         case 0x09: //SUB
         case 0x0A: //AND
         case 0x0F: //NOT
              RF.setRegister((stagenum.operand1 & 0x0C) >> 2, stagenum.result1);
              break;
         case 0x0B: //CEQ
         case 0x0C: //CLT
              break;
         case 0x0D://MUL
         case 0x0E://DIV
              RF.setRegister((stagenum.operand1 & 0x0C) >> 2, stagenum.result1);
              RF.setRegister((stagenum.operand1 & 0x03), stagenum.result2);
              break;
      }
}

bool checkDependence(Stage & stagenum)
{
      int index = findStageInQueue(stagenum.number);
      switch((int)stagenum.opcode)
      {
         case 0x00:
              //CPY
              {
                   return determineIfDependent(index, stagenum.operand1 & 0x03);
              }
              break;
         case 0x01:
              //SWAP
              {
                  if(determineIfDependent(index, stagenum.operand1 & 0x03) ||
                    (determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2)))
                    {
                        return true;
                    }
                  else
                  {
                      return false;
                  }
              }
              break;
         case 0x02:
              //LD/ST
              switch(stagenum.operand1 & 0x03)
              {
                  case 0x00:
                       //Load Immediate
                       return determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2);
                       break;
                  case 0x01:
                       //Store Immediate
                       cout<<"Store immediate -- THIS IS NOT POSSIBLE"<<endl;
                       return false;
                       break;
                  case 0x02:
                       //Load Displacement
                       if(determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2) ||
                          determineIfDependent(index, (stagenum.operand2 & 0xC0)>>6))
                          {
                               return true;
                          }
                       else
                       {
                           return false;
                       }
                       break;
                  case 0x03:
                       //Store Displacement
                       if(determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2) ||
                          determineIfDependent(index, (stagenum.operand2 & 0xC0)>>6))
                         {
                              return true;
                         }
                       else
                         {   
                             return false;
                         }
                       break;
              }
              break;
         case 0x03:
              //IN/OUT
              switch(stagenum.operand1 & 0x01)
              {
                  case 0x00:
                       //Input
                       return false;
                       break;
                  case 0x01:
                       //Output
                       //In the case of output, make sure we aren't trying to compute the register value to output
                       return determineIfDependent(index, (stagenum.operand1&0x0C)>>2);
                       break;
              }                                
              break;
         case 0x04:
              //SHIFT
              return determineIfDependent(index, (stagenum.operand1&0x0C)>>2);
              break;
         case 0x05:
              //JMP/BRANCH
              return false;
              break;
         case 0x06:
              //RET/RETI
              return false;
              break;
         case 0x07:
              //SIMD
              
              break;
         case 0x08:
              //ADD
              {
                   if(determineIfDependent(index, stagenum.operand1 & 0x03) ||
                    (determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2)))
                    {
                        return true;
                    }
                   else
                   {
                        return false;
                   }
              }
              break;
         case 0x09:
              //SUB
              {
                   if(determineIfDependent(index, stagenum.operand1 & 0x03) ||
                    (determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2)))
                    {
                        return true;
                    }
                   else
                   {
                        return false;
                   }
              }
              break;
         case 0x0A:
              //AND
              {
                   if(determineIfDependent(index, stagenum.operand1 & 0x03) ||
                    (determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2)))
                    {
                        return true;
                    }
                   else
                   {
                        return false;
                   }
              }
              break;
         case 0x0B:
              //CEQ
              if(determineIfDependent(index, stagenum.operand1 & 0x03) ||
                (determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2)))
                {
                    return true;
                }
               else
               {
                    return false;
               }
              break;
         case 0x0C:
              //CLT
              if(determineIfDependent(index, stagenum.operand1 & 0x03) ||
                (determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2)))
                {
                    return true;
                }
               else
               {
                    return false;
               }
              break;
         case 0x0D://MUL
         case 0x0E://DIV
              if(determineIfDependent(index, stagenum.operand1 & 0x03) ||
                (determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2)))
                {
                    return true;
                }
               else
               {
                    return false;
               }
              break;
         case 0x0F:
              //NOT
              if(determineIfDependent(index, stagenum.operand1 & 0x03) ||
                (determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2)))
                {
                    return true;
                }
               else
               {
                    return false;
               }
              break;
      }
}

int stallStageUntilAllOtherStagesFinished(int stagenumber)
{
      int needtostall = 0;
      for(int i = 1; i < 6; i++)
      {
              if(i != stagenumber)
              {
                   //check if the stage is complete, if not.. stall
                   
                   switch(i)
                   {
                       case 1:
                            if(s1.state != 0) needtostall = 1;
                            break;
                       case 2:
                            if(s2.state != 0) needtostall = 1;
                            break;
                       case 3:
                            if(s3.state != 0) needtostall = 1;
                            break;
                       case 4:
                            if(s4.state != 0) needtostall = 1;
                            break;
                       case 5:
                            if(s5.state != 0) needtostall = 1;
                            break;
                   }
              }
      }
      if(needtostall == 1) return 1;
      else return 0;
}
bool checkStageRegisterDependence(int snum, int rnum)
{
     switch(snum)
     {
          case 1:
               if(s1.reg1 == rnum || s1.reg2 == rnum) return true;
               break;
          case 2:
               if(s2.reg1 == rnum || s2.reg2 == rnum) return true;
               break;
          case 3:
               if(s3.reg1 == rnum || s3.reg2 == rnum) return true;
               break;
          case 4:
               if(s4.reg1 == rnum || s4.reg2 == rnum) return true;
               break;
          case 5:
               if(s5.reg1 == rnum || s5.reg2 == rnum) return true;
               break;
     }
     return false;
}

bool determineIfDependent(int index, char regnum)
{
     //we need to check from index to StagesExecuting.size whether or not the source is 
     //a register waiting to be written to as a destination of a prior command
     //if(StagesExecuting.size() == 1) return false;
     for(int i = index+1; i < StagesExecuting.size(); i++)
     {
         if(checkStageRegisterDependence(StagesExecuting[i],(int)regnum)) return true;
     }
     return false;
     
}


void resetAnyFinishedStages()
{
     if(s1.state == 6) resetStage(s1);
     if(s2.state == 6) resetStage(s2);
     if(s3.state == 6) resetStage(s3);
     if(s4.state == 6) resetStage(s4);
     if(s5.state == 6) resetStage(s5);
}

void resetStage(Stage & snum)
{     
      int tempnumber = findStageInQueue(snum.number);
      StagesExecuting.erase(StagesExecuting.begin()+tempnumber);
      snum.reset();
}

int findStageInQueue(int stagenum)
{
    for(int i = 0; i<StagesExecuting.size(); i++)
    {
            if(StagesExecuting[i] == stagenum) return i;
    }
}
void printStageQueue()
{
     cout<<"Stages Executing: "<<endl;
     for(int i = 0;i<StagesExecuting.size(); i++)
     {
             cout<<StagesExecuting[i]<<endl;
     }
}
bool allStagesComplete()
{
     if(s1.state != 0 || s2.state != 0 || s3.state != 0 || s4.state != 0 || s5.state != 0)
     {
         return false;
     }
     else
     {
         return true;
     }
}

bool anyStageStalled()
{
     if(s1.stalled == 1 || s2.stalled == 1 || s3.stalled == 1 || s4.stalled == 1 || s5.stalled == 1)
     {
         return true;
     }
     else
     {
         return false;
     }
}
bool pipelineFull()
{
     if(s1.state != 0 && s2.state != 0 && s3.state != 0 && s4.state != 0 && s5.state != 0) return true;
     else return false;     
}     
void finishPipelineExecution()
{
     //Finish executing stages we're waiting on
     if(PIPE_DEBUG && !PIPE_STALLED) cout<<"******************** Finishing Pipeline Execution *********************"<<endl;
     if(PIPE_DEBUG && PIPE_STALLED) cout<< "******************** PIPE STALLED -- Finishing Execution ****************"<<endl;
     while(!allStagesComplete())
     {              
         if(PIPE_DEBUG) cout<<"------------- MC "<<Statistics.MC_CNT<<" -------------"<<endl;
         clockPipeline();
     }
}     

void saveSystemState()
{
     DM[SP] = RF.getRegister(3);
     SP--;
     DM[SP] = RF.getRegister(2);
     SP--;
     DM[SP] = RF.getRegister(1);
     SP--;
     DM[SP] = RF.getRegister(0);
     SP--;
     DM[SP] = PC;
     SP--;
}

void restoreSystemState()
{
     SP++;
     PC = DM[SP];
     DM[SP] = 0x00;
     SP++;
     RF.setRegister(0, DM[SP]);
     DM[SP] = 0x00;
     SP++;
     RF.setRegister(1, DM[SP]);
     DM[SP] = 0x00;
     SP++;
     RF.setRegister(2, DM[SP]);
     DM[SP] = 0x00;
     SP++;
     RF.setRegister(3, DM[SP]);
     DM[SP] = 0x00;
}

void serviceInterrupt(char startloc, int priority)
{
     if(INTERRUPT_DEBUG) cout<<"SERVICING PRIORITY "<<priority<<" INTERRUPT"<<endl;
     STOP_FILLING_PIPELINE = 1;
     PC_STALLED = 1;
     finishPipelineExecution();
     saveSystemState();
     if(INTERRUPT_DEBUG)
     {
         cout<<"Saving system state before going into ISR"<<endl;
         RF.print();
         cout<<"Saved PC = "<<hex<<(int)PC<<endl;
         cout<<"Entering ISR @ "<<hex<<(int)(char)startloc<<endl;
     }
     RF.resetRegisters();
     PC = startloc;
     PC_STALLED = 0;
     PIPE_STALLED = 0;
     STOP_FILLING_PIPELINE = 0;
     INTERRUPTED = 1;
     while((PM[PC] & 0x08) != 0x08)
     {
         STOP_FILLING_PIPELINE = 0;
         clockPipeline();
         PC++;
     }
     STOP_FILLING_PIPELINE = 1;
     finishPipelineExecution();
     if(INTERRUPT_DEBUG)
     {
         cout<<"END OF PRIORITY "<<priority<<" INTERRUPT STATE:"<<endl;
         RF.print();
     }
     Statistics.RETI_CNT++;
     RF.resetRegisters();
     restoreSystemState();
     if(INTERRUPT_DEBUG)
     {
         cout<<"Restored system state:"<<endl;
         RF.print();
         cout<<"Restored PC = "<<hex<<(int)PC<<endl;
     }
     STOP_FILLING_PIPELINE = 0;
     PC_STALLED = 0;
     PIPE_STALLED = 0;
     INTERRUPTED = 0;
     
}

void registerInterrupts()
{
     if(PM[0xFF] != 0x00)
     {
         //1st priority
         serviceInterrupt(PM[0xFF], 1);
         PM[0xFF] = 0x00;
     }
     if(PM[0xFE] != 0x00)
     {
         //2nd priority
         serviceInterrupt(PM[0xFE], 2);
         PM[0xFE] = 0x00;
     }
     if(PM[0xFD] != 0x00)
     {
         //3rd priority
         serviceInterrupt(PM[0xFD], 3);
         PM[0xFD] = 0x00;
     }
     if(PM[0xFC] != 0x00)
     {
         //4th priority
         serviceInterrupt(PM[0xFC], 4);
         PM[0xFC] = 0x00;
     }
}

void printState()
{
     cout<<"Program counter = "<<hex<<(int)PC<<endl;
     cout<<"Data counter = "<<hex<<(int)DC<<endl;
     cout<<"Stack Pointer = "<<hex<<(int)SP<<endl;
     cout<<"Instruction count = "<<Statistics.I_CNT<<endl;
     RF.print();
}


void pipePrint()
{    
     
     cout<<"\tStage 1\t\tStage2\t\tStage3\t\tStage4\t\tStage5"<<endl;
     cout<<"Opcode: "<<hex<<(int)s1.opcode<<"\t\t"<<(int)s2.opcode<<"\t\t"<<(int)s3.opcode<<"\t\t"<<(int)s4.opcode<<"\t\t"<<(int)s5.opcode<<endl;
     cout<<"State : "<<stateNumToString(s1)<<"\t\t"<<stateNumToString(s2)<<"\t\t"<<stateNumToString(s3)<<"\t\t"<<stateNumToString(s4)<<"\t\t"<<stateNumToString(s5)<<endl;
     cout<<"IW1   : "<<hex<<(int)s1.operand1<<"\t\t"<<(int)s2.operand1<<"\t\t"<<(int)s3.operand1<<"\t\t"<<(int)s4.operand1<<"\t\t"<<(int)s5.operand1<<endl;
     cout<<"IW2   : "<<hex<<(int)s1.operand2<<"\t\t"<<(int)s2.operand2<<"\t\t"<<(int)s3.operand2<<"\t\t"<<(int)s4.operand2<<"\t\t"<<(int)s5.operand2<<endl;
     cout<<"Cycles: "<<dec<<s1.cyclesRemaining<<"\t\t"<<s2.cyclesRemaining<<"\t\t"<<s3.cyclesRemaining<<"\t\t"<<s4.cyclesRemaining<<"\t\t"<<s5.cyclesRemaining<<endl;
}

string stateNumToString(Stage num)
{
       if(num.cyclesRemaining != -1)
       {
           if(num.stalled == 1) { return "STALL"; }
           else
           {
               switch(num.state)
               {
                   case 0:
                        return "IF";
                        break;
                   case 1:
                        //never here
                        break;
                   case 2:
                        return "DOF";
                        break;
                   case 3:
                        return "EX";
                        break;
                   case 4:
                        return "MWB";
                        break;
                   case 5:
                        return "WB";
                        break;
                   case 6:
                        return "DONE";
                        break;
               }
           }
       }
       else
       {
           return "-";
       }
}

void loadCyclesPerInstruction(string cycle_file)
{
     ifstream infile(cycle_file.c_str());
     string opcode, cycles;
     int tempval, tempindex;
     if(infile.is_open())
     {
         while(infile.good())
         {
             getline(infile,opcode);
             getline(infile,cycles);
             tempval = strtol(cycles.c_str(),NULL,10); // convert value read in for the number of cycles to a "long" with base 10
             tempindex = strtol(opcode.c_str(),NULL,16); //use base 16 to convert hex opcode to int
             Cycles[tempindex] = tempval;
         }
         infile.close();
     }
     else
     {
         cout<<"Unable to load cycles per instruction file"<<endl;
         system("pause");
         exit(0);
     }
}

void loadProgramMemory(string mc_file)
{
    ifstream infile(mc_file.c_str());
    string line;
    int i = 0;
    unsigned char temp;
    if(infile.is_open())
    {
        while(infile.good())
        {
            getline(infile, line);
            if(line == "END") { PMEND = i; }
            else
            {
                temp = strtol((const char*)line.c_str(),NULL,16);
                PM[i] = temp;
            }
            i++;
        }
        infile.close();
    }
    else
    {
        cout<<"Unable to load Program Memory file"<<endl;
        system("pause");
        exit(0);
    } 
}

void resetFlags()
{
     C_FLAG = Z_FLAG = N_FLAG = V_FLAG = 0;
}

void printFlags()
{
     cout<<"C = "<<C_FLAG<<endl;
     cout<<"Z = "<<Z_FLAG<<endl;
     cout<<"N = "<<N_FLAG<<endl;
     cout<<"V = "<<V_FLAG<<endl;
}

unsigned char execAdd(unsigned char op1, unsigned char op2)
{
      if(((int)op1 + (int)op2) > (int)0xFF)
      {
          C_FLAG = 1;
      }
      else
      {
          C_FLAG = 0;
      }
      unsigned char temp = (op1&0x7F) + (op2&0x7F);
      if((int)(temp) > (int)0x7F)
      {
          V_FLAG = C_FLAG ^ 1;
      }
      else
      {
          V_FLAG = C_FLAG ^ 0;
      }
      if(FLAG_DEBUG)
      {
          cout<<"SET C_FLAG to "<<(int)C_FLAG<<endl;
          cout<<"SET V_FLAG to "<<(int)V_FLAG<<endl;
      }
      return (unsigned char)((int)op1 + (int)op2);
}


//Debug Functions:

void printProgramMemory()
{
     for(int i = 0;i<256; i++)
     {
          //cout<<"Address: "<<hex<<i<<" = "<<(int)PM[i]<<endl;
          printf("Address: PM[%02X] = %02X\n",i,PM[i]);
     }
}
void printDataMemory()
{
     for(int i = 0; i<256; i++)
     {
          printf("Address: DM[%02X] = %02X\n",i,DM[i]);
     }
}

void printCyclesPerInstruction()
{
     for(int i = 0; i<16; i++)
     {
             cout<<"Instruction "<<i<<" takes "<<Cycles[i]<<" cycles"<<endl;
     }
}

/*
Function testRegisters()
         Runs tests on the Register File Handling Class
*/
void testRegisters()
{
     cout<<"TESTING REGISTER FILE CLASS"<<endl;
     Registers registerfile(register_file);
     registerfile.setRegister(0,0x2B); //2B is a plus sign
     registerfile.setRegister(1,0x2C);
     registerfile.setRegister(2,0x2D);
     registerfile.setRegister(3,0x2E);
     registerfile.print();
     registerfile.writeToDisk();
     cout<<"Wrote register file to disk"<<endl;
     registerfile.resetRegisters();
     registerfile.print();
     cout<<"Reset.. now read from disk:"<<endl;
     registerfile.readFromDisk();
     registerfile.print();
     cout<<"DONE WITH TESTING OF REGISTER FILE CLASS"<<endl;
}
