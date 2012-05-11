:busy;
JMP :busy;
END;
:TXISR;
JMP :busy;
:RXISR;
IN R0,00; get size of code in R0
IN R1,01; get status from TX proc (holds what type of data to expect)
LDI R2,00; 
IN R3,00; get starting address of code
ST R0,R2,20; store size of code in DM[0x20]
ST R3,R2,21; store starting location of code in DM[0x21]
CEQ R1,R2; if status == 0, we are expecting program memory data
BRZ :PMRW; read in program memory data and jump to start loc
LDI R2,01; 
CEQ R1,R2; if status == 1, we are expecting data memory
BRZ :DMRW; read in data memory, save it and wait for the program
JMP :busy; go back to busy loop.. this shold never hit
:PMRW; R0 has size of code, R3 has start address
IN R1,00; read byte of transmssion
STI R1,R3,00; store byte to PM[R3]
LDI R2,01; 
ADD R3,R2; add 1 to address to store to
SUB R0,R2; subtract 1 from bytes to read
LDI R2,00;
CEQ R0,R2;
BRZ :gotocode;
JMP :PMRW;
:DMRW;
JMP :busy;
:gotocode;
LDI R0,00;
LDD R1,R0,21; load starting location of code to R1
JMPR R1; jump to starting location