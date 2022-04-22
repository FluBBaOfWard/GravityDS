#include <nds.h>


#ifndef SHIP_HEADER
#define SHIP_HEADER


typedef struct {
	u32 xPos;						// positions are 16.16 fixed point.
	u32 yPos;
	int xVel;						// Current velocity.
	int yVel;
	u32 direction;					// Current direction the ship is facing.
	u32 mass;						// Mass of ship.
	u32 torque;						// Power of the engine.
	int energy;						// "Health" of ship.
	u32 resistance;					// "Air"-resistance.
} ship;



void shipInit(ship * myShip);
void shipUpdatePos(ship * myShip);
void shipThrust(ship * myShip);
void shipRotate(ship * myShip, int rotSpeed);
void shipTurnRight(ship * myShip);
void shipTurnLeft(ship * myShip);

#endif
