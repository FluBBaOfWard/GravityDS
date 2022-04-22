
#include "ship.h"
#include "caveexterns.h"

extern signed char sinTable[256];
extern uint16 *playMap;
extern int scorePickUp;


//---------------------------------------------------------------------------------
void shipInit(ship *myShip) {
	myShip->xPos = 0x800000;
	myShip->yPos = 0x800000;
	myShip->xVel = 0;
	myShip->yVel = 0;
	myShip->direction = 0;
	myShip->mass = 0x10000;
	myShip->torque = 0x10000;
	myShip->energy = 10;
	myShip->resistance = 0x10000;
}

void shipUpdatePos(ship *myShip) {
	u32 oldX = myShip->xPos;
	u32 oldY = myShip->yPos;
	u32 xOfs;
	u32 yOfs;
	int mapOfs;
	int tileValue, x,y;
	int crash = 0;

	myShip->yVel += 0x1300;					// Gravity.

	myShip->xVel -= (myShip->xVel>>5);		// Air resistance.
	myShip->yVel -= (myShip->yVel>>5);

	myShip->xPos += myShip->xVel;			// Calculate new position.
	myShip->yPos += myShip->yVel;

	xOfs = myShip->xPos>>19;
	yOfs = (myShip->yPos>>19)*256;
	mapOfs = xOfs+yOfs;
	if( (tileValue = playMap[mapOfs] & 0x3FF) ) {
		if( tileValue == 0x3E8 ) {
			playMap[mapOfs] = 0;
			for(y=0;y<3;y++) {
				for(x=0;x<3;x++) {
					if( (playMap[mapOfs+x-1 + (y-1)*256] & 0x3FF) == 0x3E8) playMap[mapOfs+x-1 + (y-1)*256] = 0;
				}
			}
			scorePickUp = 1;
		} else {

			if( ((playMap[mapOfs+256] & 0x3FF) && (playMap[mapOfs-256] & 0x3FF)) ||
				!(playMap[(oldX>>19)+yOfs] & 0x3FF) ) {
				myShip->xVel = -(myShip->xVel>>1);
				myShip->xPos = oldX;
				if( abs(myShip->xVel>>15) > 3 ) crash = 1;
			}
			if( ((playMap[mapOfs+1] & 0x3FF) && (playMap[mapOfs-1] & 0x3FF)) ||
				!(playMap[xOfs+(oldY>>19)*256] & 0x3FF) ) {
				myShip->yVel = -(myShip->yVel>>1);
				myShip->yPos = oldY;
				if( abs(myShip->yVel>>15) > 3 ) crash = 1;
			}

		}
		if(crash) myShip->energy -= 1;
	}
	

	if( myShip->xPos < 0x00200000) {		// Check left side collision
		myShip->xPos = 0x00200000;
		myShip->xVel = abs(myShip->xVel>>1);
	}

	if( myShip->xPos > 0x07E00000) {		// Check right side collision
		myShip->xPos = 0x07E00000;
		myShip->xVel = -abs(myShip->xVel>>1);
	}

	if( myShip->yPos < 0x00200000) {		// Check top side collision
		myShip->yPos = 0x00200000;
		myShip->yVel = abs(myShip->yVel>>1);
	}

	if( myShip->yPos > 0x07E00000) {		// Check bottom side collision
		myShip->yPos = 0x07E00000;
		myShip->yVel = -abs(myShip->yVel>>1);
	}

}

void shipThrust(ship *myShip) {
	myShip->xVel += (int)(((int)sinTable[(myShip->direction>>24)])*myShip->torque)>>9;
	myShip->yVel += (int)(((int)sinTable[((myShip->direction+0x40000000)>>24)])*myShip->torque)>>9;
}

void shipRotate(ship *myShip, int rotSpeed) {
	myShip->direction += rotSpeed;
}

void shipTurnRight(ship *myShip) {
	shipRotate(myShip, 0xFE000000);
}

void shipTurnLeft(ship *myShip) {
	shipRotate(myShip, 0x02000000);
}

