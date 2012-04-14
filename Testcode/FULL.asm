*PM will load with ISR start address at 0xFF (1st priority interrupt vector)

:main
    LD R0, #1      ;   <-- R0 = 1
    CPY R1, R0     ;   <-- R1 = 1
    ADD R0, R1     ;   <-- R0 = 2
    SWAP R1, R0    ;   <-- R1 = 2, R0 = 1;
    OUT R1, 0x44   ;   <-- IO[0x44] = 2;
    LD R2, #2      ;   <-- R2 = 2;
    ADD R2, R0     ;   <-- R2 = 2 + 1 = 3
    LD R3, #9      ;   <-- R3 = 9
    SUB R3, R2     ;   <-- R3 = 9 - 3 = 6
    AND R3, R2     ;   <-- R3 = 6 & 3 = 2
    CEQ R3, R1     ;   <-- R3 == R2 --> 1 == 1 --> Z = 1
    NOT R3         ;   <-- R3 = 0xFD
    CLT R2, R3     ;   <-- 3 < 0xFD --> N = 1
    CALL testSHIFT ;   <-- system state stored
    JMP control    ;


:control
    LD R0, 0xFF   ;   <-- R0 = 0xFF
    LD R1, 0xFF   ;   <-- R1 = 0xFF
    IN  R2, 0x44   ;   <-- R2 = IO[0x44] = 2
    ADD R0, R1    ;   <-- R0 = 0xFF+0xFF = FE w/ C = 1
    BRC control2  ;
    LD R0, 0x00   ;   SHOULD NEVER BE HERE
    LD R1, 0x00   ;   SHOULD NEVER BE HERE
    

:control2
    LD R2, 0xAA       ;    <-- R2 = 0xAA
    ST R2, R2, 0x00   ;    <-- DM[0xAA] = 0xAA
    LD R1, 0xA0       ;    <-- R1 = 0x00
    END


:testSHIFT
    LD R0,#1    ;   <-- R0 = 1;
    SHLA R0     ;   <-- R0 = 2;
    SHLL R0     ;   <-- R0 = 4;
    LD R1,0xFF  ;   <-- R1 = 0xFF;
    SHRA R1     ;   <-- R1 = 0xFF;
    LD R2,0xFF  ;   <-- R2 = 0xFF;
    SHRL R2     ;   <-- R2 = 0x7F;
    SHRA R2     ;   <-- R2 = 0x3F;
    RET
    
    
:ISR
    LD R0, #1   ;  <-- R0 = 1
    LD R1, #2   ;  <-- R1 = 2
    ADD R0, R1  ;  <-- R0 = 3
    RETI        ;

