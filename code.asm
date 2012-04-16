:main;
LDI R0,06;
LDI R1,03;
ADD R0,R1;
JMP :test;
:test;
LDI R2,04;
SUB R0,R2;
END;