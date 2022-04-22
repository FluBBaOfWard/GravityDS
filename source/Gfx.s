#include "nds_asm.h"

	.global scrollValues
	.global paletteInit
	.global calcDMABuff
	.global vblIrqHandler
	.global DMA0BUFPTR

	.arm
	.align 4
	.section .text

;@----------------------------------------------------------------------------
vdcInit:	;@(called from main.c) only need to call once
;@----------------------------------------------------------------------------

	mov r1,#0xffffff00			;@build chr decode tbl
//	ldr r2,=CHR_DECODE
ppi0:
	mov r0,#0
	tst r1,#0x01
	orrne r0,r0,#0x10000000
	tst r1,#0x02
	orrne r0,r0,#0x01000000
	tst r1,#0x04
	orrne r0,r0,#0x00100000
	tst r1,#0x08
	orrne r0,r0,#0x00010000
	tst r1,#0x10
	orrne r0,r0,#0x00001000
	tst r1,#0x20
	orrne r0,r0,#0x00000100
	tst r1,#0x40
	orrne r0,r0,#0x00000010
	tst r1,#0x80
	orrne r0,r0,#0x00000001
	str r0,[r2],#4
	adds r1,r1,#1
	bne ppi0


	bx lr

;@ called by ui.c:  void map_palette(char gammavalue)
;@----------------------------------------------------------------------------
paletteInit:			;@ r0-r3,r12 modified.
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,lr}
	mov r12,#0xE0
//	ldr r6,=MAPPED_RGB
	ldrb r1,gammaValue			;@ gamma value = 0 -> 4
	mov r4,#1024				;@ pce rgb, r1=R, r2=G, r3=B
	sub r4,r4,#2
noMap:							;@ map 0000000gggrrrbbb  ->  0bbbbbgggggrrrrr
	and r0,r12,r4,lsl#1			;@ Red ready
	bl gPrefix
	mov r5,r0

	and r0,r12,r4,lsr#2			;@ Green ready
	bl gPrefix
	orr r5,r5,r0,lsl#5

	and r0,r12,r4,lsl#4			;@ Blue ready
	bl gPrefix
	orr r5,r5,r0,lsl#10

	strh r5,[r6,r4]
	subs r4,r4,#2
	bpl noMap



	mov r12,#0x7
//	ldr r6,=MAPPED_BNW
	mov r4,#1024				;@ pce rgb, r1=R, r2=G, r3=B
	sub r4,r4,#2
noMap2:							;@ map 0000000gggrrrbbb  ->  0bbbbbgggggrrrrr
	and r2,r12,r4,lsr#4			;@ Red ready
	ldr r3,=0x0AF8AF8A			;@ 30%
	mul r0,r3,r2

	and r2,r12,r4,lsr#7			;@ Green ready
	ldr r3,=0x1593BFA2			;@ 59%
	mla r0,r3,r2,r0

	and r2,r12,r4,lsr#1			;@ Blue ready
	ldr r3,=0x0405D9F7			;@ 11%
	mla r0,r3,r2,r0
	
	mov r0,r0,lsr#24

	bl gammaConvert
	orr r5,r0,r0,lsl#5
	orr r5,r5,r0,lsl#10

	strh r5,[r6,r4]
	subs r4,r4,#2
	bpl noMap2

	ldmfd sp!,{r4-r6,lr}
	bx lr

;@----------------------------------------------------------------------------
gPrefix:
	orr r0,r0,r0,lsr#3
	orr r0,r0,r0,lsr#6
;@----------------------------------------------------------------------------
gammaConvert:		;@ takes value in r0(0-0xFF), gamma in r1(0-4),returns new value in r0=0x1F
;@----------------------------------------------------------------------------
	rsb r2,r0,#0x100
	mul r3,r2,r2
	rsbs r2,r3,#0x10000
	subne r2,r2,#0x2a8			;@ Tweak for Gamma #4...
	rsb r3,r1,#4
	orr r0,r0,r0,lsl#8
	mul r2,r1,r2
	mla r0,r3,r0,r2
	mov r0,r0,lsr#13

	bx lr
;@----------------------------------------------------------------------------
gammaValue:
	.byte 0,0,0,0

;@----------------------------------------------------------------------------
calcDMABuff:			;@ in r0 = *dst
	.type calcDMABuff STT_FUNC
;@----------------------------------------------------------------------------
	mov r1,#192
calcLoop:
	str r1,[r0],#4
	subs r1,r1,#1
	bne calcLoop

	bx lr

;@----------------------------------------------------------------------------
	.section .itcm
vblIrqHandler:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r7,lr}

	mov r6,#REG_BASE
	ldr r0,=scrollValues		;@ buffer for scrolling:

	ldmia r0,{r1-r4}
	add r5,r6,#REG_BG0HOFS		;@ get base for scroll regs
	stmia r5,{r1-r4}

exit_vbl:
	ldmfd sp!,{r4-r7,pc}


;@----------------------------------------------------------------------------
PaletteTxAll:		;@ Called from ui.c
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r7}
	mov r3,#0x400
	sub r4,r3,#2				;@ mask=0x3FE
//	ldr r1,=PCEPALBUFF
	ldr r5,MappedColorPtr
//	ldr r2,=PCE_PALETTE
;@----------------------------------------------------------------------------
PalCpy:
	subs r3,r3,#4
	ldr r0,[r2,r3]				;@ Source
	and r7,r4,r0,lsl#1
	ldrh r7,[r5,r7]				;@ Gamma
	and r0,r4,r0,lsr#15
	ldrh r0,[r5,r0]				;@ Gamma
	orr r0,r7,r0,lsl#16
	str r0,[r1,r3]				;@ Destination
	bhi PalCpy					;@ loop to PalCpy

	ldmfd sp!,{r4-r7}
	bx lr
MappedColorPtr:
	.long 0//MAPPED_RGB
;@----------------------------------------------------------------------------

;@----------------------------------------------------------------------------
DMA0BUFPTR:			.long 0

scrollBuff:			.long 0
dmaScrollBuff:		.long 0

bgrBuff:			.long 0
dmaBgrBuff:			.long 0

pceVdcBuffer:		.long 0	;@ 1->2->1.. (loop)
tmpVdcBuffer:		.long 0	;@ pceVdc->tmpVdc->dmaVdc
dmaVdcBuffer:		.long 0	;@ triple buffered!!!

oamBufferReady:		.long 0
pcePaletteReady:	.long 0

scrollValues:
					.long 2
					.long 0
					.long 0
					.long 0
;@----------------------------------------------------------------------------

wTop:
	.long 0,0,0	;@ windowTop  (this label too)   L/R scrolling in unscaled mode
	.long 0		;@ hCenter

vdcState:
	.long 0		;@ palettePtr
	.long 0		;@ vram_w_adr
	.long 0		;@ vram_r_adr (temp)
	.long 0		;@ readLatch
	.long 0		;@ timCycles
	.long 0		;@ rasterCompare
	.long 0		;@ rasterCompareCPU
	.long 0		;@ scrollX
	.long 0		;@ scrollY
	.long 0		;@ satPtr
	.long 0		;@ sprite0y
	.long 0		;@ dmaSource
	.long 0		;@ dmaDestination
	.long 0		;@ vdcVdw

;@----------------------------------------------------------------------------
