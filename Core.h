#ifndef CORE_H
#define CORE_H
#include <iostream>
#include <sstream>
#include <deque>
#include "Assembler.h"
#include "Registers.h"
#include "Stats.h"
#include "Stage.h"
using namespace std;

class Core
{
  public:
         /*    DEBUG FLAG DEFS       */
        bool PIPE_DEBUG;
        bool PRINT_FINAL_STATE;
        bool PRINT_PC;
        bool CONTROL_DEBUG;
        bool INTERRUPT_DEBUG;
        bool FLAG_DEBUG;
        bool REGISTER_DEBUG;
        bool DEBUG_DFWD;
        bool RW_DEBUG;
        /*    END DEBUG FLAG DEFS    */
        
        /*  Files to be loaded:  */
        string register_file;
        string opcode_file;
        string cycles_file;
        string machine_code_file;
        /*  END FILE DEFS        */
        
        unsigned char PM[256]; //Program Memory Space
        unsigned char DM[256]; //Data Memory Space
        unsigned char IO[256]; //IO Memory Space
        unsigned char PC; // Program Memory Counter
        unsigned char DC; // Data Memory Counter
        unsigned char SP; // Stack Pointer
        unsigned int Cycles[16]; //Cycles per instruction (index is opcode)
        Stage Pipeline[5];// = {Stage(1), Stage(2), Stage(3), Stage(4), Stage(5)};
        
        deque <int> StagesExecuting;
        
        /*    GLOBAL FLAG DEFS      */
        bool PIPE_STALLED;
        int STOP_FILLING_PIPELINE;
        int PC_STALLED;
        int INTERRUPTED;
        bool PIPE_FULL;
        bool END_REACHED;
        int Z_FLAG, N_FLAG, C_FLAG, V_FLAG;
        int RUNNING;
        int RUN_FOR;
        int PMEND;
        int instructionClocked;
        bool SPECULATE;
        int I_SPECULATE_CNT;
        bool RUN_TO_COMPLETION_ASKED;
        bool wroteToIO,readFromIO;
        unsigned char IOADDR;
        unsigned char RXISRADDR,TXISRADDR;
        int Interrupts[4];
        bool savedStateToServiceInterrupt;
        /*     END GLOBAL FLAG DEFS   */
        
        
        
        /* GLOBAL VAR DEFS */
        Registers RF; //Create a register file
        Stats Statistics; //Create a statistics object
        /* END GLOBAL VAR DEFS */
        
        Core::Core(string, string, unsigned char, unsigned char);
        Core::Core();
        int Core::clockCore();
         
         /* FUNCTION PROTOTYPE DEFS */
        int Core::InterruptRegistered();
        bool Core::checkForPriorOutput(int);
        int Core::getDFWDstatenum(Stage);
        void Core::testRegisters(); //Prototype for register class test -- debug code
        void Core::printState(); //Debug code
        void Core::loadCyclesPerInstruction(string); //Prototype to initialize the Cycles array
        void Core::printCyclesPerInstruction(); //Debug code
        void Core::loadProgramMemory(string); //Loads program memory from a file
        void Core::printProgramMemory(); //Prints program memory 
        void Core::printDataMemory(); //Prints data memory
        void Core::pipePrint(); //Prints the state of the pipeline
        void Core::clockPipeline(); //Clocks a machine cycle of the pipeline
        void Core::clockStage(Stage&); //Clocks a machine cycle of a given stage
        void Core::DOF(Stage&); //Data operand fetch
        bool Core::checkForDataForward(Stage &, int, int, int);
        void Core::forwardData(Stage &, Stage &, int, int);
        void Core::iF(Stage&); // Instruction Fetch
        void Core::execute(Stage&); //Execution
        unsigned char Core::execAdd(unsigned char,unsigned char); //Do ADD Instruction on given operands
        void Core::MWB(Stage&); //Memory Writeback
        void Core::WB(Stage&); //Register Writeback
        void Core::resetFlags(); //Reset C,V,Z,N
        void Core::printFlags(); //Print Flags
        void Core::saveSystemState(); //Saves system state to the stack
        void Core::restoreSystemState(); //Restores system state from the stack
        bool Core::checkDependence(Stage&); //Checks if opcode of a stage is dependent on a previous operation
        bool Core::allStagesComplete(); //Returns whether or not all pipeline stages are complete
        void Core::finishPipelineExecution(); //Services all instructions in pipeline until pipeline empty
        int Core::findStageInQueue(int); //Finds the index of a stage in the instruction issued queue
        bool Core::determineIfDependent(int, char); //Called by checkDependence, actually does the register dependence checking
        void Core::resetAnyFinishedStages(); //Checks if a pipeline stage needs to be reset
        void Core::resetStage(Stage&); //Resets a pipeline stage
        int Core::stallStageUntilAllOtherStagesFinished(int);  //Returns stall value -- used in accounting when trying to stall a stage until all other stages have completed
        void Core::registerInterrupts(); //Checks the interrupt vectors and calls ISR if necessary
        void Core::serviceInterrupt(char, int); //Services an interrupt
        string Core::stateNumToString(Stage); //Converts a stage's state number to a string (iF, DOF, WB, etc)
        bool Core::anyStageStalled(); //checks if any stage in the pipeline is stalled
        bool Core::pipelineFull(); //checks if the pipeline is full of instructions
        void Core::printStageQueue(); //For debug.. prints the pipeline stage queue (used to determine data dependence)
        bool Core::checkStageRegisterDependence(int,int); //Checks to see if a register number is in use by given stage number (for determining data dependence)
        bool Core::WAW(int, int);
        /* END FUNCTION PROTOTYPE DEFS */
  private:
      
};
#endif
