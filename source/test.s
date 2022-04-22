
   .global mixSound
   .global checkInput
   .global Bin2BCD


	.arm
	.align 4
	.section .itcm

squarecounter:
	.long 0x00000534
;@-----------------------------------------------------------------------------
checkInput:
;@-----------------------------------------------------------------------------
	stmfd sp!,{lr}
	blx keysHeld
	cmp r0,#0
	beq .end
	mov r11,r11
.end:
	ldmfd sp!,{lr}
	bx lr

;@----------------------------------------------------------------------------
Bin2BCD:		;@ called from ui.c
;@----------------------------------------------------------------------------
	stmfd sp!,{r4}

	mov r1,#0
	adr r3,BCD_table			;@ BCD
	ldr r12,=0x11111110			;@ carry count mask value

Bin2BCD_loop:
	ldr r2,[r3],#4				;@ BCD (read BCD table)
	movs r0,r0,lsr#1			;@ test bit

	bcc Bin2BCD_loop_check
Bin2BCD_calc:
	add r4,r2,r1				;@ r4 = r2 + r1
	eor r2,r1,r2				;@ r2 = r1 XOR r2
	eor r2,r4,r2				;@ r2 = r4 XOR r2
	bic r2,r12,r2				;@ r2 = 0x11111110 AND NOT r2
	orr r2,r2,r2,lsr#1			;@ r2 = r2 OR (r2 LSR 1)
	sub r1,r4,r2,lsr#2			;@ r1 = r4 -  (r2 LSR 2)

Bin2BCD_loop_check:
	bne Bin2BCD_loop
	mov r0,r1

	ldmfd sp!,{r4}
	bx lr

BCD_table:
	.long 0x06666667
	.long 0x06666668
	.long 0x0666666a
	.long 0x0666666e
	.long 0x0666667c
	.long 0x06666698
	.long 0x066666ca
	.long 0x0666678e
	.long 0x066668bc
	.long 0x06666b78
	.long 0x0666768a
	.long 0x066686ae
	.long 0x0666a6fc
	.long 0x0666e7f8
	.long 0x0667c9ea
	.long 0x06698dce
	.long 0x066cbb9c
	.long 0x067976d8
	.long 0x068c87aa
	.long 0x06b8a8ee
	.long 0x076aebdc
	.long 0x086fd7b8
	.long 0x0a7fa96a
	.long 0x0e9eec6e
	.long 0x1cddd87c
	.long 0x39bbaa98
	.long 0x6d76eeca


;@-----------------------------------------------------------------------------
;@ r0=length, r1=pointer
mixSound:
;@-----------------------------------------------------------------------------
//	mov r11,r11
	mov r12,r0
	ldr r2,squarecounter
soundLoop:
	add r2,r2,r2,lsl#16
	movs r3,r2,asr#16
	mvnmi r3,r3
	mov r3,r3,lsl#1
	eor r3,r3,#0x8000
	mov r3,#0
	strh r3,[r1],#2
//	mov r3,#0
	strh r3,[r1],#2
	subs r12,r12,#1
	bne soundLoop

	str r2,squarecounter
	bx lr
	.ltorg

