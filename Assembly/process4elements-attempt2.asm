LDI R0,00; this code starting address at 0x50
LDI R1,00;
ST R0,R1,11; store i=0 at 0x11, elements are stored from A0-A3
:loop
LDI R1,11; load address of i
LDD R0,R1,00; R0 = i
LDI R2,A0; load start address of elements
ADD R2,R0; add i to start address of elements
LDD R3,R2,00; R3 = element to operate on
LDI R1,35; scalar
MUL R3,R1;
ST R3,R2,00; store result
LDI R1,01;
ADD R0,R1; i = i + 1
LDI R1,04;
CEQ R0,R1; if i = 4 we've done all the elements
BRZ :done;
LDI R1,00;
ST R0,R1,11; save new i
JMP :loop;
END;