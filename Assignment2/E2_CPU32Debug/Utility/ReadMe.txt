This directory contains:

asm32.exe  - This will assemble a source code file to a binary file in motorola format.  For example,
             If you drag & drop the file tst.s on asm32.exe then it will produce tst.mot.  It will
             also produce a listing file tst.lst.

convert.exe - This will convert a motorola binary file into a .coe file for the Xilinx coregen 
              utility and a .mif file for simulation.  For example, if you drag & drop tst.mot 
              from above on to convert.exe it will produce tst.coe and tst.mif.  tst.mif should
              be copied into the simulation directory and renamed to codememory.mif when simulating
              with Modelsim.  The .coe file may be used with coregen to regenerate the codememory when
              creating the actual hardware.

doit.bat  - batch file that runs the above assembler and converter on the file codememory.s and
copies the relevent files to the simulation and synthesis directories.

Notes on the assembler:

The assembler accepts instructions of the following forms (no spaces between operands):
   
   ; Ra <- Rb op Rc
   add Ra,Rb,Rc
   sub Ra,Rb,Rc
   and Ra,Rb,Rc
   or  Ra,Rb,Rc
   eor Ra,Rb,Rc

   ; Ra <- op Rc 
   swap  Ra,Rc      ; ALU ignores Rb, swaps top and bottom halves of register

   ; Ra <- R0 op Rc (pseudo instructions)
   mov   Ra,Rc      ; => add Ra,R0,Rc
   neg   Ra,Rc      ; => sub Ra,R0,Rc

   ; Ra <- Rb op #hhhh
   add Ra,Rb,#hhhh
   sub Ra,Rb,#hhhh
   and Ra,Rb,#hhhh
   or  Ra,Rb,#hhhh
   eor Ra,Rb,#hhhh

   ; Ra <- op #hhhh
   movh Ra,#hhhh   ; moves hhhh to the top half of register, lower half cleared

   ; Ra <- R0 op #hhhh (pseudo instructions)
   mov   Ra,#hhhh   ; => either add Ra,R0,#hhhh, or sub Ra,R0,#-hhhh

   ; writes to memory
   st  Rb,hhhh(Ra)
   st  Rb,hhhh
   st  Rb,(Ra)

   ; reads from memory
   ld  Rb,hhhh(Ra)
   ld  Rb,hhhh
   ld  Rb,(Ra)

   ; jumps (indexed, absolute)
   jmp hhhh(Ra)
   jmp hhhh
   jmp (Ra)

   ; return from subroutine
   rts    ; equivalent to jmp (R31)

   ; branch - PC relative
   bra label

   ; branch to subroutine, PC saved in R31 - PC relative
   bsr label

   ; conditional branches - PC relative
   bcs  label  ; branch if carry set (CY=1)
   bcc  label  ; branch if carry clear (CY=0)
   
   beq  label  ; branch if equal to zero (a=b or Z=1)
   bne  label  ; branch if not equal to zero (a<>b or Z=0)

   bvs  label  ; branch if overflow set (V=1)
   bvc  label  ; branch if overflow clear (V=0)

   bmi  label  ; branch if minus (negative) (N=1)
   bpl  label  ; branch if plus (positive) (N=0)

   blo  label  ; branch if low (a<b unsigned)
   bhs  label  ; branch if higher or same (a>=b unsigned)  

   bls  label  ; branch if less than or same (a<=b unsigned)
   bhi  label  ; branch if higher than (a>b unsigned)

   blt  label  ; branch if less than (a<b signed)
   bge  label  ; branch if greater than or equal (a>=b signed)

   ble  label  ; branch if less than or equal (a<b signed)
   bgt  label  ; branch if greater than (a>b signed)

The assembler accepts directives of the following forms:
   
    org  hhhh              ; origin - sets assembly location

    dc.b value,value ....  ; places byte (8-bit)  values in memory
    dc.w value,value ....  ; places word (16-bit) values in memory
    dc.l value,value ....  ; places long (32-bit) values in memory

label equ value       ; assigns a value to a label (constant!)


Expressions may be used for labels, hhhh or value in the above.  The 
assembler accepts basic C-style expressions and numbers eg 0x33 may
be used for a hex number.

Note: To place an arbitrary instruction in the memory work out the 
bit pattern and then use the following to insert that longword in memory:

      dc.l  0x12345678  ; places hex number in memory.
