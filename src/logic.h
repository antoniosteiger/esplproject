#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "input.h"
#include "objects.h"
#include <SDL2/SDL_scancode.h>
#include "timers.h"
#include "TUM_Ball.h"


#define mainGENERIC_STACK_SIZE ((unsigned short)2560)
#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)


static TaskHandle_t vEmitKeysHandle;
static TaskHandle_t vOnEnterHandle;
static TimerHandle_t welcomeDuration;

extern gameparammutex parameters;
extern statechar state;
extern hs highscores;
extern struct game game;
extern struct isgameactive isgameactive;

int initLogic();
void vOnEnter();
void vEmitKeys();
void vInitGame();
void resumeGame();
void exitGame();
int handleStartScreen(uint32_t keybits);
int handleWelcomeScreen(uint32_t keybits);
int handleHighscoreScreen(uint32_t keybits);
int handleGameScreen(uint32_t keybits);
int handlePauseScreen(uint32_t keybits);
int handleCheatsScreen(uint32_t keybits);
void switchState(int targetstate);
void onPlayerHit(void* args);
struct gameparameters getParameters();