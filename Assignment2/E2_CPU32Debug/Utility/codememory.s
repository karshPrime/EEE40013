         org   0x0000
  
; Tutorial Example
	mov r1,#0x33
	mov r1,#0x52
	add r13,r2,r1
	and r13,r2,r1
	or  r13,r2,r1
	add r13,r2,#-12
  
; Example 1- Check basic addition
         mov   R1,#100
         mov   R2,#200
         add   R3,R1,R2
incLoop1
         bra   incLoop1
	    
; Example 2 - Counting loop
;  Just a dummy loop to execute
;  R1 should be counting
;
         mov   R1,#0
         mov   R2,#1
incLoop2
         add   R1,R1,R2
         bra   incLoop2
         
; Example 2 - Check BEQ
         mov   R1,#0x33 ;  <==> add r1,r0,#0x33
         mov   R2,#0x33
         sub   R1,R1,R2
         beq   ok1
  
; End up here on error
fail1    bra   *
         mov   R0,R0
         mov   R0,R0
         mov   R0,R0
         
; End up here on success
; Can chain tests
ok1      bra   *
         mov   R0,R0
         mov   R0,R0
         mov   R0,R0
         
