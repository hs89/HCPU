:busy;
JMP :busy;
END;
:TXISR;
JMP :busy;
:RXISR;
IN R0,00;
IN R1,01;
LDI R2,00;
IN R3,00;
CEQ R1,R2;
BRZ :PMRW;
LDI R2,01;
CEQ R1,R2;
BRZ :DMRW;
JMP :busy;
:PMRW;
IN R1,00;
STI R1,R3;
LDI R2,01;
ADD R3,R2;
SUB R0,R2;
LDI R2,00;
CEQ R0,R2;
BRZ 00;
JMP :PMRW;
:DMRW;
JMP :busy;