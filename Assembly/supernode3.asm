:main;
    CALL :initDMTX;
    LDI R0,06; sending 6 bytes (less size and address) 4 are data, 2 are blank
    LDI R1,A0; store at location A0-A3
    LDI R2,2F; dec 47
    LDI R3,2B; dec 43
    CALL :sendtoP0;
    LDI R0,29; dec 41
    LDI R1,25; dec 37
    CALL :sendtoP0;
    CALL :doneTXP0;
    CALL :initDMTX;
    LDI R0,06; 
    LDI R1,A0; 
    LDI R2,1F; dec 31
    LDI R3,1D; dec 29
    CALL :sendtoP1;
    LDI R0,17; dec 23
    LDI R1,13; dec 19
    CALL :sendtoP1;
    CALL :doneTXP1;
    CALL :initDMTX;
    LDI R0,06;
    LDI R1,A0;
    LDI R2,11; dec 17
    LDI R3,0D; dec 13
    CALL :sendtoP2;
    LDI R0,0B; dec 11
    LDI R1,07; dec 7
    CALL :sendtoP2;
    CALL :doneTXP2;
    CALL :initDMTX;
    LDI R0,06;
    LDI R1,A0;
    LDI R2,05; dec 5
    LDI R3,03; dec 3
    CALL :sendtoP3;
    LDI R0,02; dec 2
    LDI R1,01; dec 1;
    CALL :sendtoP3;    
    CALL :doneTXP3;
    JMP :wait;
:wait;
    JMP :wait;
:TXISR;
    JMP :wait; will never have a tx isr because super uploads code to slaves through main function
:RXISR; must be recieving code from a slave
:initPMTX;
    LDI R0,00;
    OUT R0,04;
    RET;
:initDMTX;
    LDI R0,01;
    OUT R0,04;
    RET;
:doneTXP0;
    LDI R0,01;
    OUT R0,0A;
    RET;
:doneTXP1;
    LDI R0,01;
    OUT R0,0B;
    RET;
:doneTXP2;
    LDI R0,01;
    OUT R0,0C;
    RET;
:doneTXP3;
    LDI R0,01;
    OUT R0,0D;
    RET;
:sendtoP0;
    OUT R0,00;
    OUT R1,00;
    OUT R2,00;
    OUT R3,00;
    RET;
:sendtoP1;
    OUT R0,01;
    OUT R1,01;
    OUT R2,01;
    OUT R3,01;
    RET;
:sendtoP2;
    OUT R0,02;
    OUT R1,02;
    OUT R2,02;
    OUT R3,02;
    RET;
:sendtoP3;
    OUT R0,03;
    OUT R1,03;
    OUT R2,03;
    OUT R3,03;
    RET;
:sendtoall;
    CALL :sendtoP0;
    CALL :sendtoP1;
    CALL :sendtoP2;
    CALL :sendtoP3;
    RET;
END;