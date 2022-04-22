
#include "Ship.h"

extern ship player1;
extern int gameState;
extern int score;
extern int highScore;
extern int multiplier;
extern int gameTime;

void gameInit(void);
void gameReset(void);
void gameLoop(void);
void playerControl(void);
void calculateScore(void);
