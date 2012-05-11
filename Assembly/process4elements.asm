LDI R0,A0;
LDD R1,R0,00; load first element
LDI R3,35; scalar
MUL R1,R3;
ST R1,R0,00; store result 1
LDD R1,R0,01; load second element
LDI R3,35;
MUL R1,R3;
ST R1,R0,01; store result 2
LDD R1,R0,02; load third element
LDI R3,35;
MUL R1,R3;
ST R1,R0,02; store result 3
LDD R1,R0,02; load fourth element
LDI R3,35; 
MUL R1,R3;
ST R1,R0,03; store result 4
JMP 00;
END;