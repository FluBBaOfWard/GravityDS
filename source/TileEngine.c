#include <nds.h>


#include "Ship.h"
#include "TileEngine.h"
#include "caveexterns.h"

#define xOffset 0x00800000
#define yOffset 0x00600000
#define xLimit  (0x08000000-xOffset)
#define yLimit  (0x08000000-yOffset)

extern u16 scrollValues[8];			// gfx.s
extern signed char sinTable[256];
extern uint16 *map1;
extern uint16 *map2;
extern uint16 *map3;
uint16 *playMap;

int screenXPos = 0x800000;
int screenYPos = 0x600000;

//---------------------------------------------------------------------------------

void tileEngineInit() {
	scrollValues[0] = 0;
	scrollValues[1] = 0;
	playMap = malloc(0x20000);
}

void tileEngineReset() {
	scrollValues[0] = 0;
	scrollValues[1] = 0;
	memcpy(playMap, caveMap0,0x20000);
}

void scrollMap(int wantedXPos, int wantedYPos) {
	int finalX, finalY;
	int deltaX = wantedXPos - screenXPos;
	int deltaY = wantedYPos - screenYPos;

	screenXPos += (deltaX>>3);
	screenYPos += (deltaY>>3);
	if (screenXPos < xOffset) screenXPos = xOffset;
	if (screenYPos < yOffset) screenYPos = yOffset;
	if (screenXPos > xLimit) screenXPos = xLimit;
	if (screenYPos > yLimit) screenYPos = yLimit;
	finalX = (screenXPos - xOffset)>>16;
	finalY = (screenYPos - yOffset)>>16;

	scrollValues[2] = finalX;
	scrollValues[3] = finalY;

	scrollValues[4] = (finalX>>1);
	scrollValues[5] = (finalY>>1);

	scrollValues[6] = (finalX>>2);
	scrollValues[7] = (finalY>>2);

	outputTiles();
}

void displayShip(ship *myShip) {
	int finalX;
	int finalY;

	finalX = (((myShip->xPos - screenXPos) + xOffset)>>16) - 16;
	finalY = (((myShip->yPos - screenYPos) + yOffset)>>16) - 16;

	OAM[0] = (finalY & 0xFF) | 0x100;
	OAM[1] = (finalX & 0x1FF) | 0x8000;
	OAM[2] = 0;					// Tile number & more


	OAM[3] = sinTable[((myShip->direction+0xC0000000)>>24)]*3;		// PA
	OAM[7] = sinTable[((myShip->direction)>>24)]*3;					// PB
	OAM[11] = -sinTable[((myShip->direction)>>24)]*3;				// PC
	OAM[15] = sinTable[((myShip->direction+0xC0000000)>>24)]*3;		// PD
}

void outputTiles() {
	int x,y;
	u32 posX = (screenXPos - xOffset)>>19;
	u32 posY = (screenYPos - yOffset)>>19;

	for (y=0;y<25;y++) {
		for (x=0;x<33;x++) {
			if ( !((posX+x)&0x20) ) {
				map1[((posX+x)&0x1F) + ((posY+y)&0x1F)*32] = playMap[posX+x + (posY+y)*256] | 0x4000;
			}
			else {
				map1[((posX+x)&0x1F)+0x400 + ((posY+y)&0x1F)*32] = playMap[posX+x + (posY+y)*256] | 0x4000;
			}
		}
	}

	posX >>= 1;
	posY >>= 1;
	for (y=0;y<25;y++) {
		for (x=0;x<33;x++) {
			if ( !((posX+x)&0x20) ) {
				map2[((posX+x)&0x1F) + ((posY+y)&0x1F)*32] = caveMap1[posX+x + (posY+y)*143] | 0x4000;
			}
			else {
				map2[((posX+x)&0x1F)+0x400 + ((posY+y)&0x1F)*32] = caveMap1[posX+x + (posY+y)*143] | 0x4000;
			}
		}
	}

	posX >>= 1;
	posY >>= 1;
	for (y=0;y<25;y++) {
		for (x=0;x<33;x++) {
			if ( !((posX+x)&0x20) ) {
				map3[((posX+x)&0x1F) + ((posY+y)&0x1F)*32] = caveMap2[posX+x + (posY+y)*87] | 0x4000;
			}
			else {
				map3[((posX+x)&0x1F)+0x400 + ((posY+y)&0x1F)*32] = caveMap2[posX+x + (posY+y)*87] | 0x4000;
			}
		}
	}
}
