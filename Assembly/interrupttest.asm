:main;
LDI R0,09;
STI R0,FF;
LDI R0,00;
LDI R1,00;
END;
LDI R0,00;
LDI R1,01;
LDI R2,02;
LDI R3,03;
RETI;
:TXISR;
:RXISR;