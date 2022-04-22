#include <nds.h>
#include <fat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <maxmod9.h>
#include "GameLogic.h"
#include "Ship.h"
#include "EmuFont.h"
#include "Ship1.h"
#include "caveexterns.h"

extern mm_word mixSound( mm_word length, mm_addr dest, mm_stream_formats format );
extern int Bin2BCD(int value);
extern void vblIrqHandler(void);
extern void calcDMABuff(int *destination);

extern int *DMA0BUFPTR;

void setupStream(void);
void setupGraphics(void);
void waitVBlank(void);
void testFAT(void);
void drawtext(const char *str, int, int);
void drawTextMain(const char *str, int, int);
void cls(int chrmap);
void strmerge(char *dst, const char *src1, const char *src2);
void hex2str(char *dest, int hexValue);
void hex2strByte(char *dest, int hexValue);
void drawGUI(void);


#define sample_rate  31536
#define buffer_size  (528)*2
int squarecounter = 0x00000534;

static bool vBlankOverflow = false;

static mm_ds_system sys;
static mm_stream mystream;

uint16 *map0 = (uint16*)SCREEN_BASE_BLOCK(0);
uint16 *map1 = (uint16*)SCREEN_BASE_BLOCK(2);
uint16 *map2 = (uint16*)SCREEN_BASE_BLOCK(4);
uint16 *map3 = (uint16*)SCREEN_BASE_BLOCK(6);
uint16 *map0sub = (uint16*)SCREEN_BASE_BLOCK_SUB(0);
uint16 *map1sub = (uint16*)SCREEN_BASE_BLOCK_SUB(1);


//---------------------------------------------------------------------------------
void myVblank(void) {
//---------------------------------------------------------------------------------
	vBlankOverflow = true;
	vblIrqHandler();
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	setupGraphics();

	DMA0BUFPTR = malloc(192*4);
	calcDMABuff(DMA0BUFPTR);
	setupStream();
	irqSet(IRQ_VBLANK, myVblank);

	cls(0);
	drawtext("Testing the drawroutines!!",10,0);
//	testFAT();
	gameInit();
	gameReset();
	while (1) {
		scanKeys();
		drawGUI();
		gameLoop();
		waitVBlank();
	}
	return 0;
}

//---------------------------------------------------------------------------------
void waitVBlank(void) {
//---------------------------------------------------------------------------------
	// Workaround for bug in Bios.
	if (!vBlankOverflow) {
		swiIntrWait(1, IRQ_VBLANK);
	}
	vBlankOverflow = false;
}

//---------------------------------------------------------------------------------
void setupGraphics(void) {
//---------------------------------------------------------------------------------
	int ix = 0;
	int iy = 0;


	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	vramSetBankC(VRAM_C_MAIN_BG_0x06040000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06060000);
	vramSetBankE(VRAM_E_MAIN_SPRITE);
	vramSetBankF(VRAM_F_LCD);
	vramSetBankG(VRAM_G_LCD);
	vramSetBankH(VRAM_H_SUB_BG);
	vramSetBankI(VRAM_I_SUB_SPRITE);

	// Set up the main display
	videoSetMode(MODE_0_2D | 
					DISPLAY_SPR_1D_LAYOUT | 
					DISPLAY_SPR_ACTIVE | 
					DISPLAY_BG0_ACTIVE |
					DISPLAY_BG1_ACTIVE |
					DISPLAY_BG2_ACTIVE |
					DISPLAY_BG3_ACTIVE |
					DISPLAY_CHAR_BASE(1));
	
	// Set up the sub display
	videoSetModeSub(MODE_0_2D | 
					DISPLAY_SPR_1D_LAYOUT | 
					DISPLAY_SPR_ACTIVE | 
					DISPLAY_BG0_ACTIVE |
					DISPLAY_BG1_ACTIVE );

	// Set up backgrounds to scroll around
	REG_BG0CNT = BG_32x32 | BG_MAP_BASE(0) | BG_COLOR_16 | BG_TILE_BASE(2) | BG_PRIORITY(0);
	REG_BG1CNT = BG_64x32 | BG_MAP_BASE(2) | BG_COLOR_16 | BG_TILE_BASE(0) | BG_PRIORITY(0);
	REG_BG2CNT = BG_64x32 | BG_MAP_BASE(4) | BG_COLOR_16 | BG_TILE_BASE(0) | BG_PRIORITY(0);
	REG_BG3CNT = BG_64x32 | BG_MAP_BASE(6) | BG_COLOR_16 | BG_TILE_BASE(0) | BG_PRIORITY(0);

	// Set up two backgrounds for menu
	REG_BG0CNT_SUB = BG_32x32 | BG_MAP_BASE(0) | BG_COLOR_16 | BG_TILE_BASE(0) | BG_PRIORITY(0);
	REG_BG1CNT_SUB = BG_32x32 | BG_MAP_BASE(1) | BG_COLOR_16 | BG_TILE_BASE(0) | BG_PRIORITY(0);
	REG_BG1HOFS_SUB = 0;
	REG_BG1VOFS_SUB = 0;

	// Clear map0
	for (iy = 0; iy < 32; iy++) {
		for (ix = 0; ix <32; ix++) {
			map0[iy * 32 + ix] = 0;	//(((ix ^ iy) & 8)>>3) | 0x2000;
		}
	}

	memcpy(BG_GFX+0x8000, caveTiles, 0x8000);
	decompress(EmuFontTiles, BG_GFX+0xD200, LZ77Vram);

	decompress(EmuFontTiles, BG_GFX_SUB+0x1200, LZ77Vram);
	decompress(Ship1Tiles, SPRITE_GFX, LZ77Vram);
	memcpy(BG_PALETTE_SUB, EmuFontPal, 64);
	memcpy(BG_PALETTE, EmuFontPal, 64);
	memcpy(BG_PALETTE+64, cavePalette, 64*2);
	memcpy(SPRITE_PALETTE, Ship1Pal, 16*2);
	BG_PALETTE[0x00] = RGB15(0,0,0);
	BG_PALETTE[0x20] = RGB15(0,0,0);
	BG_PALETTE[0x21] = RGB15(0,31,0);
	BG_PALETTE[0x22] = RGB15(31,0,0);
	BG_PALETTE[0x23] = RGB15(0,0,31);
	BG_PALETTE[0x24] = RGB15(15,0,15);
}

//---------------------------------------------------------------------------------
void setupStream(void) {
//---------------------------------------------------------------------------------

	//----------------------------------------------------------------
	// initialize maxmod without any soundbank (unusual setup)
	//----------------------------------------------------------------
//	mm_ds_system sys;
	sys.mod_count 			= 0;
	sys.samp_count			= 0;
	sys.mem_bank			= 0;
	sys.fifo_channel		= FIFO_MAXMOD;
	mmInit( &sys );

	//----------------------------------------------------------------
	// open stream
	//----------------------------------------------------------------
//	mm_stream mystream;
	mystream.sampling_rate	= sample_rate;				// sampling rate =
	mystream.buffer_length	= buffer_size;				// buffer length =
	mystream.callback		= mixSound;					// set callback function
	mystream.format			= MM_STREAM_16BIT_STEREO;	// format = stereo 16-bit
	mystream.timer			= MM_TIMER0;				// use hardware timer 0
	mystream.manual			= false;					// use manual filling
	mmStreamOpen( &mystream );

	//----------------------------------------------------------------
	// when using 'automatic' filling, your callback will be triggered
	// every time half of the wave buffer is processed.
	//
	// so: 
	// 25000 (rate)
	// ----- = ~21 Hz for a full pass, and ~42hz for half pass
	// 1200  (length)
	//----------------------------------------------------------------
	// with 'manual' filling, you must call mmStreamUpdate
	// periodically (and often enough to avoid buffer underruns)
	//----------------------------------------------------------------
}

void testFAT(void) {
	char str[32];
	int i=0;

	if (fatInitDefault()) {
		DIR *pdir;
		struct dirent *pent;
		struct stat statbuf;

		pdir=opendir("/");

		if (pdir) {
			while ((pent=readdir(pdir))!=NULL) {
	    		stat(pent->d_name,&statbuf);
				if (strcmp(".", pent->d_name) == 0 || strcmp("..", pent->d_name) == 0) {
	        		continue;
				}
	    		if (S_ISDIR(statbuf.st_mode)) {
					strmerge(str, pent->d_name, " <dir>");
	        		drawtext(str,i,0);
				}
				else {
	        		drawtext(pent->d_name,i,0);
				}
				i++;
			}
			closedir(pdir);
		}
		else {
			drawtext("opendir() failure.",0,0);
		}
	}
	else {
		drawtext("fatInitDefault() failure.",0,0);
	}
}

//---------------------------------------------------------------------------------
void drawGUI(void) {
//---------------------------------------------------------------------------------
	int i;
	char txttmp[64];

	if(gameState == 3) {
		drawTextMain("           Game Over",11,0);
	} else if(gameState == 0) {
		drawTextMain("          Press Start",11,0);
	} else if(gameState == 2) {
		drawTextMain("             Paused",11,0);
	} else {
		drawTextMain("                    ",11,0);
	}
	strcpy(txttmp,"Score:");
	hex2str(&txttmp[6], score);
	strmerge(txttmp, txttmp, "     High:");
	hex2str(&txttmp[24], highScore);
	drawTextMain(txttmp,0,0);

	if (multiplier > 1) {
		strcpy(txttmp,"            Combo:");
		txttmp[18] = (multiplier & 0xF) + 0x30;
		txttmp[19] = 'X';
		txttmp[20] = 0;
		drawTextMain(txttmp,5,0);
	} else {
		drawTextMain("                 ",5,0);
	}

	strcpy(txttmp,"Time:");
	hex2strByte(&txttmp[5], gameTime/60);
	strmerge(txttmp, txttmp, "                         ");
	for(i=0; i<player1.energy; i++) {
		txttmp[31-i] = 'I';
	}
	txttmp[32] = 0;
	drawTextMain(txttmp,23,0);

}

void drawTextMain(const char *str,int row,int hilite) {
	u16 *here = map0+row*32;
	int i = 0;

	*here = hilite?0x012a:0x0120;
	hilite = (hilite<<12)+0x0100;
//	here++;
	while (str[i] >= ' ') {
		here[i] = str[i]|hilite;
		i++;
		if (i > 31) break;
	}
	for (; i<32; i++) {
		here[i] = 0x0120;
	}
}

void drawtext(const char *str,int row,int hilite) {
	u16 *here = map0sub+row*32;
	int i = 0;

	*here=hilite?0x012a:0x0120;
	hilite=(hilite<<12)+0x0100;
//	here++;
	while (str[i] >= ' ') {
		here[i] = str[i]|hilite;
		i++;
		if (i>31) break;
	}
	for (; i<32; i++) {
		here[i]=0x0120;
	}
}

void strmerge(char *dst,const char *src1,const char *src2) {
	if(dst!=src1)
		strcpy(dst,src1);
	strcat(dst,src2);
}

void cls(int chrmap) {
	int i=0,len=0x400;
	u32 *scr=(u32*)map0sub;
	for (; i<len; i++) {			//512x256
		scr[i]=0x01200120;
	}
}

void hex2str(char * dest, int hexValue) {
	int i, v;
	v = Bin2BCD(hexValue);
	for (i=0; i<8; i++) {
		dest[7-i] = (v & 0xF) + 0x30;
		v = v>>4;
	}
	dest[8] = 0;
}

void hex2strByte(char * dest, int hexValue) {
	int i, v;
	v = Bin2BCD(hexValue);
	for (i=0; i<2; i++) {
		dest[1-i] = (v & 0xF) + 0x30;
		v = v>>4;
	}
	dest[2] = 0;
}

