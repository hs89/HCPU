:main;    
    LDI R0,00; 
    LDI R1,01; 
    ST R1,R0,01; x[1] = 1
    LDI R1,02;
    ST R1,R0,02; x[2] = 2
    LDI R1,03;
    ST R1,R0,03; x[3] = 3
    LDI R1,05;
    ST R1,R0,04; x[4] = 5
    LDI R1,07;
    ST R1,R0,05; x[5] = 7
    LDI R1,0B;
    ST R1,R0,06; x[6] = 11
    LDI R1,0D;
    ST R1,R0,07; x[7] = 13
    LDI R1,11; 
    ST R1,R0,08; x[8] = 17
    LDI R1,13;
    ST R1,R0,09; x[9] = 19
    LDI R1,17;
    ST R1,R0,0A; x[10] = 23
    LDI R1,1D; 
    ST R1,R0,0B; x[11] = 29
    LDI R1,1F;
    ST R1,R0,0C; x[12] = 31
    LDI R1,25;
    ST R1,R0,0D; x[13] = 37
    LDI R1,29;
    ST R1,R0,0E; x[14] = 41
    LDI R1,2B; 
    ST R1,R0,0F; x[15] = 43
    LDI R1,2F; 
    ST R1,R0,10; x[16] = 47
    LDI R2,10; i = 16
    ST R2,R0,11; store i at DM[0+0x11]
    LDI R0,00;
    LDI R1,00;
    LDI R2,00;
    LDI R3,00;
    JMP :mulloop;
:mulloop;
    LDI R0,00; load base address of data structure
    LDD R1,R0,11; load i into R1
    LDD R2,R1,00; R2 = x[i]
    LDI R3,35; R3 = 53
    MUL R2,R3; R2 and R3 = x[i] * 53
    ST R3,R1,00; x[i] = x[i] * 53
    LDI R3,01;
    SUB R1,R3; i = i-1;
    ST R1,R0,11; store new i
    CLT R0,R1; if i>0 -> N = 0
    BRN :mulloop;
END;