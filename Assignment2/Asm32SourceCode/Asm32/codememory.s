         org   0x0000
  
   swap R1,R2
   movh R1,#0x1234

   mov  R1,R2
   mov  R1,#0x1234

   add  R1,R2
   sub  R1,R2
   and  R1,R2
   or   R1,R2
   eor  R1,R2
   xor  R1,R2
   mul  R1,R2

   add  R1,#0x1234
   sub  R1,#0x1234
   and  R1,#0x1234
   or   R1,#0x1234
   eor  R1,#0x1234
   xor  R1,#0x1234
   mul  R1,#0x1234

   add  R1,R2,R3
   sub  R1,R2,R3
   and  R1,R2,R3
   or   R1,R2,R3
   eor  R1,R2,R3
   xor  R1,R2,R3
   mul  R1,R2,R3

   add  R1,R2,#0x1234
   sub  R1,R2,#0x1234
   and  R1,R2,#0x1234
   or   R1,R2,#0x1234
   eor  R1,R2,#0x1234
   xor  R1,R2,#0x1234
   movh R1,#0x1234
   mul  R1,R2,#0x1234

   ld  R1,0x1234(R2)
   ld  R1,(R2)
   ld  R1,0x1234
   jmp 0x1234(R2)
   jmp 0x1234
   jmp (R2)
   rts

   st  R1,0x1234(R2)
   st  R1,(R2)
   st  R1,0x1234

label:
   bra label
   bcs label
   blo label
   beq label
   bvs label

   bmi label
   blt label
   ble label
   bls label

   bsr label
   bcc label
   bhs label
   bne label
   bvc label

   bpl label
   bge label
   bgt label
   bhi label


; Tutorial Example
   mov r1,#0
   mov r2,#1
loop1:
   add r1,r1,r2
   bra loop1

