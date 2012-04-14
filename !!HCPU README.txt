Holden Sandlar
Advanced Computer Architecture
HCPU - MIPS SIMULATOR

On Start the following data is loaded into the simulator:

*	Program Memory is loaded from the file "MACHINE_CODE"
*	Cycles/Instruction is loaded from the file "cyclesperinstruction"
*	Opcodes are loaded from the file "opcodes"
* 	Registers are loaded from the file "registerfile"

Information about my Insruction Set Architecture (ISA)

ISA:
0 - CPY
1 - SWAP
2 - LD
2 - ST
3 - IN
3 - OUT
4 - SHLA
4 - SHLR
4 - SHRA
4 - SHRL
5 - JMP
5 - BR
5 - CALL
6 - RET
6 - RETI
7 - SIMD (NOT YET IMPLEMENTED)
8 - ADD
9 - SUB
A - AND
B - CEQ
C - CLT
D - MUL (NOT YET IMPLEMENTED)
E - DIV (NOT YET IMPLEMENTED)
F - NOT

The development environment used for this project was Bloodshed Dev-C++


*Simulator Commands:
-----------------
Control Commands 
-----------------
(exit || q) - quit simulator
(help || ?) - show this help menu
n           - clock next machine cycle
ni <number> - clock <number> machine cycles
c           - clock machine cycles automatically to end of program

-----------------
Debug Commands   
-----------------
pdf - PRINT TOGGLE FLAGS STATUS'
ppc - Print PC Debug TOGGLE
pd -  Pipe Debug TOGGLE
pfc - Print final statistics TOGGLE
pcd - Print control debug TOGGLE
pid - Print interrupt debug TOGGLE
pfd - Print flag debug TOGGLE
prd - Print register debug TOGGLE

-----------------
Info Commands    
-----------------
pp  - Print Pipeline info
ps  - print statistics
pr  - print registers
pf  - print Flags
pi  - print general info
ppm - Print Program Memory
pdm - Print Data Memory

