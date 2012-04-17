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
bool DEBUG_DFWD = true;
bool DEBUG_ROB = true;
/*    END DEBUG FLAG DEFS    */

/*  Files to be loaded:  */
string register_file = "registerfile";
string opcode_file = "opcodes";
string cycles_file = "cyclesperinstruction";
string code_file = "Assembly/loop.asm";
string machine_code_file = "MACHINE_CODE";
/*  END FILE DEFS        */

unsigned char PM[256]; //Program Memory Space
unsigned char DM[256]; //Data Memory Space
unsigned char IO[256]; //IO Memory Space
unsigned char PC = 0x00; // Program Memory Counter
unsigned char DC = 0x00; // Data Memory Counter
unsigned char SP = 0xFF; // Stack Pointer
unsigned int Cycles[16]; //Cycles per instruction (index is opcode)
Stage Pipeline[5] = {Stage(1),Stage(2),Stage(3),Stage(4),Stage(5)};

deque <int> StagesExecuting;
deque <Stage> ROB;

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
bool RUN_TO_COMPLETION_ASKED = false;
/*     END GLOBAL FLAG DEFS   */

bool SPECULATE = false;
char SAVED_PC;
bool TAKE_BRANCH = true;
bool WRITE_BACK_ROB = false;


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
bool checkForDataForward(Stage &, int, int, int);
void forwardData(Stage &, Stage &, int, int);
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
void printROB();
void clearSpeculativeFromPipeline();
/* END FUNCTION PROTOTYPE DEFS */

/* GLOBAL VAR DEFS */
Registers RF(register_file); //Create a register file
Stats Statistics; //Create a statistics object
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
           else if(command == "pstage")
           {
                int tempinput;
                cin>>tempinput;
                Pipeline[tempinput-1].print();
                continue;
           }
           else if(command == "psq")
           {
                printStageQueue();
                continue;
           }
           else if(command == "prob")
           {
                printROB();
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
                RUN_TO_COMPLETION_ASKED = true;
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
                     if(!RUN_TO_COMPLETION_ASKED)
                     {      
                         cout<<"************************** PROGRAM END REACHED **************************"<<endl;
                         cout<<"Would you like to automatically finish the rest of execution? (y/n): ";
                         string autofinish;
                         cin>>autofinish;
                         RUN_TO_COMPLETION_ASKED = true;
                         if(autofinish.at(0) == 'Y' || autofinish.at(0) == 'y')
                         {            
                              finishPipelineExecution();
                              RUNNING = 1;
                              continue;                    
                         }           
                     }
                     finishPipelineExecution();         
                     PC_STALLED = 1;
                     continue;
                }
                else if(PC == PMEND && allStagesComplete())
                {
                     cout<<"************************** PROGRAM END REACHED **************************"<<endl;
                     PC_STALLED = 1;
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
     int alreadyClocked[5] = {0,0,0,0,0};
     Statistics.MC_CNT++;
     
     if(!WRITE_BACK_ROB)
     {
         if(StagesExecuting.size() > 0)
         {
             for(int j = StagesExecuting.size()-1;j>0;j--)
             {
                     //Clock stages already in pipeline starting from the end before clocking in the next instruction
                     clockStage(Pipeline[StagesExecuting[j]-1]);
                     alreadyClocked[StagesExecuting[j]-1] = 1;
                     if(STOP_FILLING_PIPELINE == 1) instructionClocked = 1;
                     resetAnyFinishedStages();
             }
         }
         
         for(int i = 0;i<5;i++)
         {
                 if(alreadyClocked[i] != 1)
                 {
                     clockStage(Pipeline[i]);
                     if(STOP_FILLING_PIPELINE == 1) instructionClocked = 1;
                     resetAnyFinishedStages();
                 }
         }
     }
     else
     {
         if(ROB.size() > 0)
         {
             PC_STALLED = true;
             //Any stage in the ROB is in mem writeback (state 4)
             ROB.back().speculate = 0;
             clockStage(ROB.back());
             clockStage(ROB.back());
             //ROB.back().print();
             ROB.pop_back();
             if(DEBUG_ROB) cout<<"Committed another ROB instruction"<<endl;
         }
         else
         {
             //even though we are done writing back the ROB there may still be instructions in the pipeline
             //which were due to speculation..
             //make sure to service these before we turn the program counter back on and starting clocking in new
             //instructions
             SPECULATE = false;
             int clocked_pipe_spec = 0;
             for(int x = StagesExecuting.size()-1; x>=0; x--)
             {
                     if(Pipeline[StagesExecuting[x]-1].speculate == 1)
                     {
                         Pipeline[StagesExecuting[x]-1].speculate = 0;
                         clockStage(Pipeline[StagesExecuting[x]-1]);
                         Pipeline[StagesExecuting[x]-1].speculate = 1;
                         clocked_pipe_spec = 1;
                         if(STOP_FILLING_PIPELINE == 1) instructionClocked = 1;
                         resetAnyFinishedStages();
                     }
             }
             if(clocked_pipe_spec == 0)
             {
                  //there are no more speculative insructions to deal with
                  PC_STALLED = false;
                  WRITE_BACK_ROB = false;
             }
             
         }
     }
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
              if(SPECULATE) stagenum.speculate = true;
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
              cout<<"In the clockStage switch for instruction Fetch ... how? iF already happened"<<endl;
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
              if(stagenum.speculate != 1)
              {
                  //Stage in Memory Writeback
                  MWB(stagenum);
                  stagenum.cyclesRemaining--;
                  stagenum.state++;
              }
              else
              {
                  //This is speculative.. push this stage into the ROB
                  //make sure to advance this instruction so this stage of the pipeline gets cleared
                  //pipePrint();
                  //printStageQueue();
                  if(stagenum.speculate == 1)
                  {
                      if(DEBUG_ROB) cout<<"Pushing stage "<<stagenum.number<<" into the reorder buffer"<<endl;
                      ROB.push_front(stagenum);
                      stagenum.state += 2; // This makes state = 6 and stage will get reset
                  }
              }
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
                  stagenum.hasop2 = true;
                  break;
             case 0x01:
                  //SWAP
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  stagenum.reg2 = (int)((stagenum.operand1&0x03));
                  break;
             case 0x02:
                  //LD/ST
                  if((stagenum.operand1 & 0x01) == 0x00)
                  {
                      //Load into a register -- mark destination register
                      stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                      stagenum.hasop2 = true;
                  }
                  break;
             case 0x03:
                  //IN/OUT
                  if(stagenum.opcode & 0x01 == 0x00)
                  {
                      //IN -- mark destination register
                      stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                      stagenum.hasop2 = true;
                  }
                  STOP_FILLING_PIPELINE = 1;
                  PC_STALLED = 1;
                  break;
             case 0x04:
                  //SHIFT
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  stagenum.hasop2 = true;
                  break;
             case 0x05:
                  //JMP/BR/CALL
                  //In the case of a jump,branch we want to continue executing speculatively
                  //It is important to save the current PC value in case we speculated incorrectly
                  //It is also important to set the speculate flag to ensure any instruction clocked into
                  //the pipeline isn't written back but instead clocked into the ROB when finished
                  
                  //STOP_FILLING_PIPELINE = 1;
                  //PC_STALLED = 1;
                  
                  SAVED_PC = PC;
                  if(TAKE_BRANCH)
                  {
                      PC = stagenum.operand2-1;           
                  }
                  
                  //If we don't take the branch we will just continue to advance the PC and execute speculatively
                  
                  
                                    
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
             case 0x08://ADD
             case 0x09://SUB
             case 0x0A://AND
             case 0x0B://CEQ
             case 0x0C://CLT
             case 0x0F://NOT
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  //stagenum.hasop2 = true;
                  break;
             case 0x0D://MUL
             case 0x0E://DIV
                  stagenum.reg1 = (int)((stagenum.operand1&0x0C)>>2);
                  stagenum.reg2 = (int)((stagenum.operand2&0x03));
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
     if(stagenum.hasop1 && stagenum.hasop2) stagenum.stalled = 0;
     //cout<<"Stage "<<stagenum.number<<" has op1: "<<stagenum.hasop1<<" and has op2: "<<stagenum.hasop2<<endl;
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
                  if(!stagenum.hasop1) stagenum.data_in1 = RF.getRegister(stagenum.opcode & 0x03);
                  stagenum.hasop1 = stagenum.hasop2 = true;
                  break;
             case 0x01:
                  //SWAP -- this op should really take 2 MC's to execute
                  if(!stagenum.hasop1) stagenum.data_in1 = RF.getRegister((stagenum.opcode & 0x0C) >> 2);
                  if(!stagenum.hasop2) stagenum.data_in2 = RF.getRegister( stagenum.opcode & 0x03);
                  stagenum.hasop1 = stagenum.hasop2 = true;
                  break;
             case 0x02:
                  //LD/ST
                  //Data in for a Load/Store is the value in Ry, or the displacement value
                  switch(stagenum.operand1 & 0x03)
                  {
                      case 0x00:
                           //Load Immediate
                           if(!stagenum.hasop1) stagenum.data_in1 = stagenum.operand2;
                           stagenum.hasop1 = stagenum.hasop2 = true;
                           break;
                      case 0x01:
                           //Store Immediate
                           //No such thing
                           break;
                      case 0x02:
                           //Load Displacement
                           if(!stagenum.hasop1) stagenum.data_in1 = RF.getRegister((stagenum.operand2 & 0xC0)>>6);
                           stagenum.data_in2 = stagenum.operand2 & 0x3F;
                           stagenum.hasop1 = stagenum.hasop2 = true;
                           break;
                      case 0x03:
                           //Store Displacement
                           if(!stagenum.hasop1) stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                           if(!stagenum.hasop2) stagenum.data_in2 = RF.getRegister((stagenum.operand2 & 0xC0)>>6);
                           stagenum.hasop1 = stagenum.hasop2 = true;
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
                           if(!stagenum.hasop1) stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                           //TODO: Come back and fix this -- this is a workaround because the stage doesn't stall properly
                           stagenum.hasop1 = stagenum.hasop2 = true;
                           break;
                  }
                  //stagenum.stalled = stallStageUntilAllOtherStagesFinished(stagenum.number);
                  break;
             case 0x04:
                  //SHIFT
                  if(!stagenum.hasop1) stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  stagenum.hasop1 = stagenum.hasop2 = true;
                  break;
             case 0x05:
                  {
                      //JMP/BR
                      if(!stagenum.hasop1) stagenum.data_in1 = stagenum.operand2;
                      
                      //we want to stall this stage now until all other stages have finished executing
                      //stagenum.stalled = stallStageUntilAllOtherStagesFinished(stagenum.number);
                      if(StagesExecuting[StagesExecuting.size()-1] != stagenum.number)
                      {
                           //Still waiting to compute a branch condition
                           //May be executing speculatively though..
                           stagenum.stalled = 1;
                           SPECULATE = true;
                           if(DEBUG_ROB) cout<<"Other Stages before JMP (stage "<<stagenum.number<<") are going.. execute speculatively"<<endl;
                      }
                      else
                      {
                          stagenum.stalled = 0;
                      }
                      
                      stagenum.hasop1 = stagenum.hasop2 = true;
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
                           if(!stagenum.hasop1) stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x03));
                           stagenum.stalled = stallStageUntilAllOtherStagesFinished(stagenum.number);
                           stagenum.hasop1 = stagenum.hasop2 = true;
                           break;
                  }
                  break;
             case 0x07:
                  //SIMD
                  //TODO: Implement Later
                  break;
             case 0x08://ADD
             case 0x09://SUB
             case 0x0A://AND
             case 0x0B://CEQ
             case 0x0C://CLT
             case 0x0D://MUL
             case 0x0E://DIV
                  if(!stagenum.hasop1) stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  if(!stagenum.hasop2) stagenum.data_in2 = RF.getRegister(stagenum.operand1 & 0x03);
                  stagenum.hasop1 = stagenum.hasop2 = true;
                  break;
             case 0x0F:
                  //NOT
                  if(!stagenum.hasop1) stagenum.data_in1 = RF.getRegister((stagenum.operand1 & 0x0C)>>2);
                  stagenum.hasop1 = stagenum.hasop2 = true;
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
                       stagenum.result1 = DM[stagenum.data_in1 + stagenum.data_in2]; //Computing MAeff
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
              if(stagenum.result1 == 0x00 && stagenum.result2 == 0x00 && stagenum.cyclesRemaining == 3) Z_FLAG = 1;
              
              break;
         case 0x0E:
              //DIV
              stagenum.result1 = (stagenum.data_in1 / stagenum.data_in2);
              stagenum.result2 = (stagenum.data_in1 % stagenum.data_in2);
              if(stagenum.result1 == 0x00 && stagenum.result2 == 0x00 && stagenum.cyclesRemaining == 3) Z_FLAG = 1;
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
                       printf("Stored %02X to DM[%02X] in stage %d\n",stagenum.result1,stagenum.result2,stagenum.number);
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
                       printf("Wrote %02X to IO[%02X]\n",stagenum.result1,stagenum.operand2);
                       break;
              }
              break;
      }
}

void WB(Stage & stagenum)
{
     bool jumped;
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
                       //printf("Loaded DM[%02X] = %02X\n",stagenum.result1, stagenum.result1);
                       RF.setRegister((stagenum.operand1 & 0x0C)>>2, stagenum.result1);
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
                       printf("Read %02X from IO[%02X]\n",RF.getRegister((stagenum.operand1&0x0C)>>2),stagenum.operand2);
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
              jumped = false;
              switch(stagenum.operand1 & 0x0F)
              {
                  case 0x00:
                       //JMP Unconditionally
                       PC = (char)((int)stagenum.result1-1); // The -1 accounts for the PC being incremented after completeion of this stage
                       jumped = true;
                       break;
                  case 0x01:
                       //BR if V = 1
                       if(V_FLAG == 1) {
                                 PC = (char)((int)stagenum.result1-1);
                                 jumped = true;
                       }
                       break;
                  case 0x02:
                       //BR if Z = 1
                       if(Z_FLAG == 1) {
                                 PC = (char)((int)stagenum.result1-1);
                                 jumped = true;
                       }
                       break;
                  case 0x04:
                       //BR if N = 1
                       if(N_FLAG == 1) {
                                 PC = (char)((int)stagenum.result1-1);
                                 jumped = true;
                       }
                       break;
                  case 0x08:
                       //BR if C = 1
                       if(C_FLAG == 1) {
                                 PC = (char)((int)stagenum.result1-1);
                                 jumped = true;
                       }
                       break;
                  case 0x0A:
                       //BR if C&Z = 1
                       if(C_FLAG == 1 && Z_FLAG == 1) {
                                 PC = (char)((int)stagenum.result1-1);
                                 jumped = true;
                       }
                       break;
                  case 0x0C:
                       //BR if C&N = 1
                       if(C_FLAG == 1 && N_FLAG == 1) {
                                 PC = (char)((int)stagenum.result1-1);
                                 jumped = true;
                       }
                       break;
                  case 0x0F:
                       //CALL
                       if(CONTROL_DEBUG)
                       {
                           cout<<"Calling a subroutine. Stored system state: "<<endl;
                           RF.print();
                           cout<<"PC Stored = "<<hex<<(int)PC<<endl;
                       }
                       jumped = true;
                       saveSystemState();
                       PC = (char)((int)stagenum.result1-1);
                       RF.resetRegisters();
                       break; 
              }
              STOP_FILLING_PIPELINE = 0; // Re-enable filling of the pipeline now that we have written back the updated PC
              PC_STALLED = 0;
              if(jumped == TAKE_BRANCH)
              {
                      //awesome... we predicted the right direction... write the ROB back
                      WRITE_BACK_ROB = true;
                      if(DEBUG_ROB) cout<<"RIGHT PREDICTION -- start writing back from ROB"<<endl;
                      SPECULATE = false;
              }
              else
              {
                  //fuck.. we predicted wrong.. clear the rob
                  //also, why not change our prediction for next time around (assuming loop)
                  if(DEBUG_ROB) cout<<"WRONG PREDICTION -- clearing ROB and any speculative instructions in pipe"<<endl;
                  ROB.clear();
                  if(TAKE_BRANCH == false) TAKE_BRANCH = true;
                  if(TAKE_BRANCH == true ) TAKE_BRANCH = false;
                  //also need to reset the PC to the correct location..
                  PC = SAVED_PC;
                  //also need to clear any currently speculating instructions from the pipeline
                  clearSpeculativeFromPipeline();
                  SPECULATE = false;
                  
              }
              //PIPE_FULL = false;
              //PIPE_STALLED = false;
              resetFlags();
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
              resetFlags();
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
      bool dep_on_op1 = false;
      bool dep_on_op2 = false;
      int op1regnum,op2regnum;
      switch((int)stagenum.opcode)
      {
         case 0x00:
              //CPY
              {
                   dep_on_op1 = determineIfDependent(index, stagenum.operand1 & 0x03);
                   if(dep_on_op1) op1regnum = (stagenum.operand1&0x03);
              }
              break;
         case 0x02:
              //LD/ST
              switch(stagenum.operand1 & 0x03)
              {
                  case 0x00:
                       //Load Immediate
                       //dep_on_op1 = determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2);
                       //if(dep_on_op1) op1regnum = ((stagenum.operand1&0x0C)>>2);
                       break;
                  case 0x01:
                       //Store Immediate
                       cout<<"Store immediate -- THIS IS NOT POSSIBLE"<<endl;
                       return false;
                       break;
                  case 0x02:
                       //Load Displacement
                       ///dep_on_op1 = determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2);
                       dep_on_op1 = determineIfDependent(index, (stagenum.operand2 & 0xC0)>>6);
                       //if(dep_on_op1) op1regnum = ((stagenum.operand1&0x0C)>>2);
                       if(dep_on_op1) op1regnum = ((stagenum.operand2&0xC0)>>6);
                       break;
                  case 0x03:
                       //Store Displacement
                       dep_on_op1 = determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2);
                       //cout<<"Looking for dependence on "<<(int)((stagenum.operand2 & 0xC0)>>6)<<endl;
                       dep_on_op2 = determineIfDependent(index, (stagenum.operand2 & 0xC0)>>6);
                       if(dep_on_op1) op1regnum = (int)((stagenum.operand1&0x0C)>>2);
                       if(dep_on_op2) op2regnum = (int)((stagenum.operand2&0xC0)>>6);
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
                       //In the case of output, make sure we aren't trying to compute the register value to output
                       dep_on_op1 = determineIfDependent(index, (stagenum.operand1&0x0C)>>2);
                       if(dep_on_op1) op1regnum = ((stagenum.operand1&0x0C)>>2);
                       break;
              }                                
              break;
         case 0x04://SHIFT
         case 0x0F://NOT
              dep_on_op1 = determineIfDependent(index, (stagenum.operand1&0x0C)>>2);
              if(dep_on_op1) op1regnum = ((stagenum.operand1&0x0C)>>2);
              break;
         case 0x05://JMP/BRANCH/CALL
         case 0x06://RET/RETI
              return false;
              break;
         case 0x07:
              //SIMD
              break;
         case 0x08://ADD
         case 0x09://SUB
         case 0x0A://AND
         case 0x0B://CEQ
         case 0x0C://CLT
         case 0x0D://MUL
         case 0x0E://DIV
         case 0x01://SWAP
              dep_on_op1 = determineIfDependent(index, (stagenum.operand1 & 0x0C)>>2);
              dep_on_op2 = determineIfDependent(index, (stagenum.operand1 & 0x03));
              if(dep_on_op1) op1regnum = ((stagenum.operand1&0x0C)>>2);
              if(dep_on_op2) op2regnum = (stagenum.operand1&0x03);
              break;
      }
      
      //At this point we know whether there is a dependency with operand1, operand2, or both
      //We also know the register number for each of them
      //Now we should go through and figure out if any data forwarding is possible
      
      if(dep_on_op1)
      {
          dep_on_op1 = checkForDataForward(stagenum, index, op1regnum, 1);
      }
      if(dep_on_op2)
      {
          //cout<<"Checking for data forward on stage "<<stagenum.number<<" with register "<<op2regnum<<endl;
          dep_on_op2 = checkForDataForward(stagenum, index, op2regnum, 2);
          //cout<<"Dep on op 2 = "<<dep_on_op2<<endl;
      }
      //cout<<"Stagenum = "<<stagenum.number<<"   Have op1 = "<<stagenum.hasop1<< " .. regnum: "<<op1regnum<<" ; Have op2 = "<<stagenum.hasop2<<"  regnum: "<<op2regnum<<endl;
      //if(stagenum.hasop1 && stagenum.hasop2) return true;
      if(dep_on_op1 || dep_on_op2) return true;
      else return false;
}

bool checkForDataForward(Stage & stagenum, int index, int regnum, int which_operand)
{
     //The return value should be false if data was forwarded -- represents no data dependency
     //for(int i = index+1; i < StagesExecuting.size(); i++)
     //cout<<"index = "<<index<< " adn queue = "<<endl;
     //printStageQueue();
     //for(int i = index+1; i>0;i--)
     for(int i = index; i<StagesExecuting.size();i++)
     {
             //cout<<"Checking stage "<<StagesExecuting[i]<<" for needed data... state = "<<Pipeline[StagesExecuting[i]-1].state<<endl;
             if(Pipeline[StagesExecuting[i]-1].state > 3)
             {
                  if(Pipeline[StagesExecuting[i]-1].reg1 == regnum ||
                     Pipeline[StagesExecuting[i]-1].reg2 == regnum  )
                  {
                      //cout<<"Waiting for stage "<<StagesExecuting[i]<<" to write back to "<<regnum<<endl;
                      //Data forward is possible
                      forwardData(stagenum, Pipeline[StagesExecuting[i]-1], regnum, which_operand);
                      stagenum.dfwd = 1;
                      return false;
                  }
                  else
                  {
                      continue;
                  }
             }
     }
     return true; //Dependency still exists
}

void forwardData(Stage & dest, Stage & origin, int regnum, int which_operand)
{
     //Basically want to write the result for regnum of origin into temp_data{which_operand} of dest
     //after we've written an operand make sure to flag dest.hasop{which_operand} as true
     //Should look at the opcode to figure out how to forward the appropriate data
     char data;
     //cout<<"Doing a data forward... the origin is in state "<<origin.state<<endl;
     switch(origin.opcode & 0xF0)
     {
          case 0x00: //CPY
               data = origin.result1;
               break;
          case 0x01: //SWAP
               if(origin.reg1 == regnum) data = origin.result2;
               if(origin.reg2 == regnum) data = origin.result1;
               break;
          case 0x02: //LD/ST
               //For LD/ST we only care about loads because they will effect the registers
               //In result1 is stored the effective memory address of the data
               if((origin.operand1 & 0x03) == 0x00) //LD IMMEDIATE
                   data = origin.result1;
               if((origin.operand1 & 0x03) == 0x02) //LD DISPLACEMENT
                   data = DM[origin.result1];
               break;
          case 0x03: //IN/OUT
               if((origin.operand1 & 0x01) == 0x01) //INPUT
                   data = origin.result1;
               break;
          case 0x04: //SHIFT
               data = origin.result1;
               break;
          case 0x05: //JMP/BR/CALL
               break;
          case 0x06: //RET/RETI
               break;
          case 0x07: //SIMD
               break;
          case 0x08: //ADD
          case 0x09: //SUB
          case 0x0A: //AND
          case 0x0F: //NOT
               data = origin.result1;
               break;
          case 0x0B: //CEQ
          case 0x0C: //CLT
               //No writeback to registers.. we dont care about this in terms of data forwarding
               break;
          case 0x0D: //MUL
          case 0x0E: //DIV
               if(origin.reg1 == regnum) data = origin.result1;
               if(origin.reg2 == regnum) data = origin.result2;
               break;
     }
     
     if(which_operand == 1)
     {
          dest.data_in1 = data;
          dest.hasop1 = true;
     }
     if(which_operand == 2)
     {
          dest.data_in2 = data;
          dest.hasop2 = true;
     }
     if(DEBUG_DFWD) cout<<"Data foward regnum "<<regnum<<" from stage "<<origin.number<<" to "<<dest.number<<" as operand "<<which_operand;
     if(DEBUG_DFWD) printf(" -- Forwarded data of %02X\n",data);
}

int stallStageUntilAllOtherStagesFinished(int stagenumber)
{
      int needtostall = 0;
      for(int i = 1; i < 6; i++)
      {
              if(i != stagenumber)
              {
                   //check if the stage is complete, if not.. stall
                   if(Pipeline[i-1].state != 0) needtostall = 1;
              }
      }
      if(needtostall == 1) return 1;
      else return 0;
}
bool checkStageRegisterDependence(int snum, int rnum)
{
     if(Pipeline[snum-1].reg1 == rnum || Pipeline[snum-1].reg2 == rnum) return true;
     else return false;
}

bool determineIfDependent(int index, char regnum)
{
     //we need to check from index to StagesExecuting.size whether or not the source is 
     //a register waiting to be written to as a destination of a prior command
     //if(StagesExecuting.size() == 1) return false;
     if(Pipeline[StagesExecuting[index]-1].speculate == 0)
     {
         for(int i = index+1; i < StagesExecuting.size(); i++)
         {
             if(checkStageRegisterDependence(StagesExecuting[i],(int)regnum)) return true;
         }
         return false; 
     }   
     /*else
     {
         cout<<"Checking data dependence for speculative instruction... "<<endl;
         //For a speculative instruction data dependence could be:
         //    - in the pipeline
         //    - in the ROB
         //we need really really really need to figure out where there may be a data dependency
         //the dependency will only exist within speculated instructions
         
     }*/
}


void resetAnyFinishedStages()
{
     for(int i = 0;i<5;i++)
     {
             if(Pipeline[i].state == 6) resetStage(Pipeline[i]);
     }
}

void resetStage(Stage & snum)
{     
      cout<<""; // don't know why but this fixes a bug...
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
     for(int i = 0;i<5;i++)
     {
             if(Pipeline[i].state != 0) return false;
     }
     return true;
}

bool anyStageStalled()
{
     for(int i = 0;i<5;i++)
     {
             if(Pipeline[i].stalled == 1) return true;
     }
     return false;
}
bool pipelineFull()
{
     if(Pipeline[0].state != 0 && Pipeline[1].state != 0 && Pipeline[2].state != 0 && Pipeline[3].state != 0 && Pipeline[4].state != 0) return true;
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
     cout<<"Opcode: "<<hex<<(int)Pipeline[0].opcode<<"\t\t"<<(int)Pipeline[1].opcode<<"\t\t"<<(int)Pipeline[2].opcode<<"\t\t"<<(int)Pipeline[3].opcode<<"\t\t"<<(int)Pipeline[4].opcode<<endl;
     cout<<"State : "<<stateNumToString(Pipeline[0])<<"\t\t"<<stateNumToString(Pipeline[1])<<"\t\t"<<stateNumToString(Pipeline[2])<<"\t\t"<<stateNumToString(Pipeline[3])<<"\t\t"<<stateNumToString(Pipeline[4])<<endl;
     cout<<"IW1   : "<<hex<<(int)Pipeline[0].operand1<<"\t\t"<<(int)Pipeline[1].operand1<<"\t\t"<<(int)Pipeline[2].operand1<<"\t\t"<<(int)Pipeline[3].operand1<<"\t\t"<<(int)Pipeline[4].operand1<<endl;
     cout<<"IW2   : "<<hex<<(int)Pipeline[0].operand2<<"\t\t"<<(int)Pipeline[1].operand2<<"\t\t"<<(int)Pipeline[2].operand2<<"\t\t"<<(int)Pipeline[3].operand2<<"\t\t"<<(int)Pipeline[4].operand2<<endl;
     cout<<"Cycles: "<<dec<<Pipeline[0].cyclesRemaining<<"\t\t"<<Pipeline[1].cyclesRemaining<<"\t\t"<<Pipeline[2].cyclesRemaining<<"\t\t"<<Pipeline[3].cyclesRemaining<<"\t\t"<<Pipeline[4].cyclesRemaining<<endl;
     cout<<"Specul: "<<dec<<Pipeline[0].speculate<<"\t\t"<<Pipeline[1].speculate<<"\t\t"<<Pipeline[2].speculate<<"\t\t"<<Pipeline[3].speculate<<"\t\t"<<Pipeline[4].speculate<<endl;
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
void clearSpeculativeFromPipeline()
{
     for(int i = 0;i<5;i++)
     {
             if(Pipeline[i].speculate == 1)
             {
                  resetStage(Pipeline[i]);               
             }
     }    
}

void printROB()
{
     for(int i = 0;i<ROB.size();i++)
     {
             ROB[i].print();
     }
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
