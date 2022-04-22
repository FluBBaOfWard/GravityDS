#include <nds.h>

#include "GameLogic.h"
#include "Ship.h"
#include "TileEngine.h"


ship player1;
int score;
int scorePickUp;
int highScore = 10000;
int comboCounter;
int comboTimer;
int comboScore;
int multiplier;
int gameState;
int gameOverTimer;
int gameTime;

//---------------------------------------------------------------------------------

void gameInit() {
	tileEngineInit();
	gameState = 0;
}

void gameReset() {
	shipInit(&player1);
	tileEngineReset();
	score = 0;
	scorePickUp = 0;
	comboCounter = 0;
	comboTimer = 0;
	comboScore = 0;
	gameOverTimer = 60*5;				// 5 seconds.
	gameTime = 60*90;					// 1.5 minutes.
}

void gameLoop() {
	playerControl();
	calculateScore();
	scrollMap(player1.xPos, player1.yPos);
	displayShip(&player1);

	if (gameState == 1) {				// Running
		if (gameTime-- < 0) {
			gameState = 3;				// GameOver
		}
	}
}

void playerControl() {
	int dpad = keysHeld();
	int pressed = keysDown();

	if (gameState == 0) {				// Not running
		if (pressed & KEY_START) {
			gameState = 1;
			pressed = 0;
			gameReset();
		}
	}

	if (gameState == 1) {				// Running
		if (pressed & KEY_START) {
			gameState = 2;
			pressed = 0;
		}
		if (dpad & KEY_LEFT)
			shipTurnLeft(&player1);
		if (dpad & KEY_RIGHT)
			shipTurnRight(&player1);
		if (dpad & KEY_B)
			shipThrust(&player1);

		shipUpdatePos(&player1);
		if (player1.energy <= 0) {
			gameState = 3;				// GameOver
		}
	}

	if (gameState == 2) {				// Paused
		if (pressed & KEY_START) {
			gameState = 1;
			pressed = 0;
		}
	}

	if (gameState == 3) {				// GameOver
		if (gameOverTimer < 60*4)
			if (pressed & (KEY_START | KEY_A | KEY_B |KEY_X|KEY_Y)) {
				gameState = 0;
				pressed = 0;
			}
		if (gameOverTimer-- <= 0)
			gameState = 0;
	}
}

void calculateScore() {

	if (scorePickUp) {
		scorePickUp = 0;
		score += 100;
		comboScore += 100;
		comboCounter += 1;
		comboTimer = 180;
	}

	multiplier = 1;
	if (comboCounter > 2) multiplier = 2;
	if (comboCounter > 4) multiplier = 3;
	if (comboCounter > 9) multiplier = 4;
	if (comboCounter > 14) multiplier = 5;
	if (comboCounter > 19) multiplier = 6;

	if (comboTimer >= 0) comboTimer--;
	if (comboTimer == 1) {
		score += multiplier * comboScore;
		comboScore = 0;
		comboCounter = 0;
	}
	
	if ( highScore < score) {
		highScore = score;
	}
}
