:main;     for (i=16; i>0; i=i–1)
LDI R0,00; x[i] = x[i] + s; where s is a scalar value of your choice
LDI R1,05; value to initialize all elements of array to
ST R1,R0,00;
ST R1,R0,01;
ST R1,R0,02;
ST R1,R0,03;
ST R1,R0,04;
ST R1,R0,05;
ST R1,R0,06;
ST R1,R0,07;
ST R1,R0,08;
ST R1,R0,09;
ST R1,R0,0A;
ST R1,R0,0B;
ST R1,R0,0C;
ST R1,R0,0D;
ST R1,R0,0E;
ST R1,R0,0F;
ST R1,R0,10;
LDI R2,10; i = 16
ST R2,R0,11; store i at DM[0+0x20]
LDI R0,00;
LDI R1,00;
LDI R2,00;
LDI R3,00;
JMP :loop;
:loop;
LDI R0,00; load base address of data structure
LDD R1,R0,11; load i into R1
LDD R2,R1,00; R2 = x[i]
LDI R3,04; scalar to be added
ADD R2,R3; R2 = x[i] + 0x04
ST R2,R1,00; x[i] = x[i] + 0x04
LDI R3,01;
SUB R1,R3; i = i-1;
ST R1,R0,11; store new i
CLT R0,R1; if i>0 -> N = 0
BRN :loop;
END;