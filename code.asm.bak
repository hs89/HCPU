:main;
    LDI R0,01      ;   <-- R0 = 1
    CPY R1,R0     ;   <-- R1 = 1
    ADD R0,R1     ;   <-- R0 = 2
    SWAP R1,R0    ;   <-- R1 = 2, R0 = 1;
    OUT R1,44   ;   <-- IO[0x44] = 2;
    LDI R2,02      ;   <-- R2 = 2;
    ADD R2,R0     ;   <-- R2 = 2 + 1 = 3
    LDI R3,09      ;   <-- R3 = 9
    SUB R3,R2     ;   <-- R3 = 9 - 3 = 6
    AND R3,R2     ;   <-- R3 = 6 & 3 = 2
    CEQ R3,R1     ;   <-- R3 == R2 --> 1 == 1 --> Z = 1
    NOT R3         ;   <-- R3 = 0xFD
    CLT R2,R3     ;   <-- 3 < 0xFD --> N = 1
    CALL :testSHIFT ;   <-- system state stored
    JMP :control    ;
:control;
    LDI R0,FF   ;   <-- R0 = 0xFF
    LDI R1,FF   ;   <-- R1 = 0xFF
    IN R2,44   ;   <-- R2 = IO[0x44] = 2
    ADD R0,R1    ;   <-- R0 = 0xFF+0xFF = FE w/ C = 1
    BRC :control2  ;
    LDI R0,00   ;   SHOULD NEVER BE HERE
    LDI R2,00   ;   SHOULD NEVER BE HERE
:control2;
    LDI R2,AA       ;    <-- R2 = 0xAA
    ST R2,R2,00   ;    <-- DM[0xAA] = 0xAA
    LDI R1,A0       ;    <-- R1 = 0xA0
    END;
:testSHIFT;
    LDI R0,01    ;   <-- R0 = 1;
    SHLA R0     ;   <-- R0 = 2;
    SHLL R0     ;   <-- R0 = 4;
    LDI R1,FF  ;   <-- R1 = 0xFF;
    SHRA R1     ;   <-- R1 = 0xFF;
    LDI R2,FF  ;   <-- R2 = 0xFF;
    SHRL R2     ;   <-- R2 = 0x7F;
    SHRA R2     ;   <-- R2 = 0x3F;
    RET ;
:ISR;
    LDI R0,01   ;  <-- R0 = 1
    LDI R1,02   ;  <-- R1 = 2
    ADD R0,R1  ;  <-- R0 = 3
    RETI        ;