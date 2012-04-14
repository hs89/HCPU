Holden Sandlar
Advanced Computer Architecture - 0301-810
Assembler for HCPU User Manual

The assembler for HCPU is built right into the executable HCPU.exe
When HCPU.exe is run it will first try to assemble the code before
dropping you at the HCPU prompt. If it fails to assemble the code
you will be informed of the line number of the assembly file
which incurred the failure.

The default location HCPU will try to read the assembly code from is "code.asm"
You may specify a different code location by passing a filename as the first parameter
when starting HCPU. Example:

"HCPU.exe Assembly/loop.asm"
^ This will attempt to assemble Assembly/loop.asm, saving the resulting machine code to file MACHINE_CODE
Assuming the assembler completes successfully, MACHINE_CODE will be loaded into HCPU's program
memory and you will be placed at the HCPU prompt.

Specifications of the Assembly:
**COMMANDS MUST BE FOLLOWED BY ONLY A SINGLE SPACE BEFORE THE OPERANDS
**EVERY LINE MUST END WITH A SEMICOLON (;)
**NO SPACES ARE ALLOWED BEFORE OR AFTER THE COMMA (,) IN THE OPERANDS
**ANY TEXT FOLLOWING THE SEMICOLON (;) IS CONSIDERED A COMMENT AND THUS IGNORED BY THE ASSEMBLER
**ANY LINE STARTING WITH A COLON (:) IS CONSIDERED A LABEL
**SPACES CAN BE ADDED AT THE BEGINNING OR END OF A LINE
**ANY NUMERIC VALUE (IMMEDIATE OR DISPLACEMENT) IS TREATED AS HEXADECIMAL
**END COMMAND MUST APPEAR SOMEWHERE IN CODE IN ORDER FOR HCPU TO STOP

The following instructions can be issued in the assembly file: (treat as examples)
CPY R0,R1; copy R1 to R0
SWAP R0,R1; swap values in R1 and R0
LDD R0,R1,00; load displacement -> R0 = DM[R1+0x00]
LDI R0,05; load immediate -> R0 = 0x05
ST R0,R1,00; store displacement -> DM[R1+0x00] = R0
IN R0,0A; R0 = IO[0x0A]
OUT R0,0A; IO[0x0A] = R0
SHLA R0; shift R0 arithmetic left
SHLL R0; shift R0 logical left
SHRA R0; shift R0 arithmetic right
SHRL R0; shift R0 logical right
JMP :label; jump conditionally to :label
BRC :label; if C = 1, jump to :label
BRN :label; if N = 1, jump to :label
BRZ :label; if Z = 1, jump to :label
BRV :label; if V = 1, jump to :label
BRCN :label; if C&N = 1, jump to :label
BRCZ :label; if C&Z = 1, jump to :label
CALL :label; call subroutine at :label
RET; returns from subroutine
RETI; returns from interrupt service routine
*SIMD; not yet implemented
ADD R0,R1; R0 = R0+R1
SUB R0,R1; R0 = R0-R1
AND R0,R1; R0 = R0&R1
CEQ R0,R1; if R0 == R1 --> Z = 1, else Z = 0
CLT R0,R1; if R0 < R1 --> N = 1, else N = 0
*MUL R0,R1; not yet implemented
*DIV R0,R1; not yet implemented
NOT R0; R0 = ~R0
END; signifies program end

For further examples of assembly code see the Assembly directory