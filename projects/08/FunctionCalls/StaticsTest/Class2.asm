(Class2.set)
@ 0
D=A
(Class2.set.localLoop)
@Class2.set.localEnd
D;JLE
@SP
M=M+1
A=M-1
M=0
D=D-1
@Class2.set.localLoop
0;JMP
(Class2.set.localEnd)
@ARG
D=M
@0
A=A+D
D=M
@SP
M=M+1
A=M-1
M=D
@SP
AM=M-1
D=M
@Class2.0
M=D
@ARG
D=M
@1
A=A+D
D=M
@SP
M=M+1
A=M-1
M=D
@SP
AM=M-1
D=M
@Class2.1
M=D
@0
D=A
@SP
M=M+1
A=M-1
M=D
@LCL
D=M
@FRAME
M=D
@5
D=A
@FRAME
A=M-D
D=M
@RET
M=D
@SP
AM=M-1
D=M
@ARG
A=M
M=D
D=A
@SP
M=D+1
@FRAME
A=M-1
D=M
@THAT
M=D
@2
D=A
@FRAME
A=M-D
D=M
@THIS
M=D
@3
D=A
@FRAME
A=M-D
D=M
@ARG
M=D
@4
D=A
@FRAME
A=M-D
D=M
@LCL
M=D
@RET
A=M
0;JMP
(Class2.get)
@ 0
D=A
(Class2.get.localLoop)
@Class2.get.localEnd
D;JLE
@SP
M=M+1
A=M-1
M=0
D=D-1
@Class2.get.localLoop
0;JMP
(Class2.get.localEnd)
@Class2.0
D=M
@SP
M=M+1
A=M-1
M=D
@Class2.1
D=M
@SP
M=M+1
A=M-1
M=D
@SP
AM=M-1
D=M
A=A-1
M=M-D
@LCL
D=M
@FRAME
M=D
@5
D=A
@FRAME
A=M-D
D=M
@RET
M=D
@SP
AM=M-1
D=M
@ARG
A=M
M=D
D=A
@SP
M=D+1
@FRAME
A=M-1
D=M
@THAT
M=D
@2
D=A
@FRAME
A=M-D
D=M
@THIS
M=D
@3
D=A
@FRAME
A=M-D
D=M
@ARG
M=D
@4
D=A
@FRAME
A=M-D
D=M
@LCL
M=D
@RET
A=M
0;JMP
