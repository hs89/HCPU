:main;
    CALL :initDMTX;
    LDI R0,06; sending 5 bytes
    LDI R1,A0; store at location A0-A3
    LDI R2,2F; dec 47
    LDI R3,2B; dec 43
    CALL :sendtoP0;
    LDI R0,29; dec 41
    LDI R1,25; dec 37
    LDI R2,00; empty
    LDI R3,00; empty
    CALL :sendtoP0;
    CALL :doneTX;
    CALL :initPMTX;
    LDI R0,20; sending 32 bytes of PM
    LDI R1,50; start PM storage at loc 0x50;
    LDI R2,20;
    LDI R3,A0;
    CALL :sendtoP0;
    LDI R0,26;
    LDI R1,00;
    LDI R2,2C;
    LDI R3,35;
    CALL :sendtoP0;
    LDI R0,D7;
    LDI R1,27;
    LDI R2,00;
    LDI R3,26;
    CALL :sendtoP0;
    LDI R0,01;
    LDI R1,2C;
    LDI R2,35;
    LDI R3,D7;
    CALL :sendtoP0;
    LDI R0,27;
    LDI R1,01;
    LDI R2,26;
    LDI R3,02;
    CALL :sendtoP0;
    LDI R0,2C;
    LDI R1,35;
    LDI R2,D7;
    LDI R3,27;
    CALL :sendtoP0;
    LDI R0,02;
    LDI R1,26;
    LDI R2,02;
    LDI R3,2C;
    CALL :sendtoP0;
    LDI R0,35;
    LDI R1,D7;
    LDI R2,27;
    LDI R3,03;
    CALL :sendtoP0;
    LDI R0,50; jump to busy wait on processing node
    LDI R1,00;
    CALL :sendtoP0;
    CALL :doneTX;
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
:doneTX;
    LDI R0,01;
    OUT R0,0A;
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
END;