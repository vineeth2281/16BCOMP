// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

// Put your code here.

(WAIT_KEY)
    @KBD
    D = M
    @FILL_BLACK
    D; JGT
@WAIT_KEY
0; JMP


(FINISHED_BLACK)
    @KBD
    D = M
    @FILL_WHITE
    D; JEQ
@FINISHED_BLACK
0; JMP

(FINISHED_WHITE)
@WAIT_KEY
0; JMP


(FILL_BLACK)
    @SCREEN
    D = A
    @R0
    M = D

    (BLACK_LOOP)
        @R0
        A = M
        M = -1
        
        @R0
        M = M + 1

        @24575
        D = A
        @R0
        D = D - M

        @BLACK_LOOP
        D; JGE
    
    @FINISHED_BLACK
    0; JMP


(FILL_WHITE)
    @SCREEN
    D = A
    @R0
    M = D

    (WHITE_LOOP)
        @R0
        A = M
        M = 0

        @R0
        M = M + 1

        @24575
        D = A
        @R0
        D = D - M

        @WHITE_LOOP
        D; JGE
    
    @FINISHED_WHITE
    0; JMP

        


