#include "logic.h"
#include "graphics.h"
#include "sound.h"
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include "TUM_Event.h"
#include "semphr.h"
#include "timers.h"
#include <SDL2/SDL_scancode.h>
#include "TUM_Ball.h"
#include "objects.h"
//#include "screenhandlers.h"
//#include "TUM_Print.h"

static TaskHandle_t vEmitKeysHandle = NULL;
static TaskHandle_t vOnEnterHandle = NULL;
static TimerHandle_t welcomeDuration = NULL;

extern gameparammutex parameters;
extern statechar state;
extern hs highscores;
extern buttons_buffer_t buttons;
extern struct game game;
extern struct isgameactive isgameactive;

// static wall_t* playerhitbox = NULL;

//###### FUNCTIONS ######//
int initLogic()
{

    

    
    if(state.lock){
        if (xSemaphoreTake(state.lock, 0) == pdTRUE) {
            state.state = 0;
            xSemaphoreGive(state.lock);
        }
    }

    if(parameters.lock){
        if (xSemaphoreTake(parameters.lock, 0) == pdTRUE) {
            parameters.level = 1;
            parameters.score = 0;
            parameters.lives = 3;
            parameters.inf_lives = 0;
            xSemaphoreGive(parameters.lock);
        }
    }

    
    if(highscores.lock){
        if(xSemaphoreTake(highscores.lock, 0) == pdTRUE) {
            for(int i = 0; i < NUMOFHIGHSCORES; i++) {
                highscores.highscores[i] = -1;
            }
            xSemaphoreGive(highscores.lock);
        }
    }

    // game.gamespeed.lock = xSemaphoreCreateMutex();

    int init_fault = !game.position.lock ||
        !game.playershot.lock;

    if(init_fault)
        exitGame();

    // if(xSemaphoreTake(game.gamespeed.lock, 0) == pdTRUE) {
    //     game.gamespeed.gamespeed = 1;
    //     xSemaphoreGive(game.gamespeed.lock);
    // }

    if(xSemaphoreTake(game.position.lock, 0) == pdTRUE) {
        game.position.position = LEFT_BORDER;
        xSemaphoreGive(game.position.lock);
    }

    if(xSemaphoreTake(game.playershot.lock, 0) == pdTRUE) {
        game.playershot.active = 0;
        xSemaphoreGive(game.playershot.lock);
    }

    
    welcomeDuration = xTimerCreate(
        "Gamestart Timer",
        pdMS_TO_TICKS(500),
        pdFALSE,
        (void*) 0,
        vInitGame
    );
    
    init_fault = !buttons.lock || !state.lock || !parameters.lock || !highscores.lock ||
            !welcomeDuration ||
            (xTaskCreate(vOnEnter, "vOnEnter", mainGENERIC_STACK_SIZE * 16,
                    NULL, (configMAX_PRIORITIES), &vOnEnterHandle) != pdPASS) ||
            (xTaskCreate(vEmitKeys, "vEmitKeys", mainGENERIC_STACK_SIZE * 16,
                    NULL, (configMAX_PRIORITIES - 1), &vEmitKeysHandle) != pdPASS);

    if(init_fault) {
        printf("Failed to initialize logic!\n");
        return -1;
    }
    else {
        printf("Logic Initialized.\n");
        return 0;
    }
}

void exitGame() 
{
    exitGraphics();
    exitSound();
    exitInput();
    exit(EXIT_SUCCESS);
}

int handleStartScreen(uint32_t keybits) 
{
    int state = 0;
    static int menuentry = 1;

    switch(keybits) {
        case ESC_BIT:
            state = -1;
            break;
        case ENTER_BIT:
            if (menuentry == 1) {
                stopGame();
                state = 5; //Enter Welcome Screen
            }
            else if (menuentry == 2)
                state = 2;
            else if (menuentry == 3)
                state = 3;
            break;
        case DOWN_BIT:
            if(menuentry < 3) {
                menuentry += 1;
                renderStartScreen(menuentry);
            }
            break;
        case UP_BIT:
            if(menuentry > 1) {
                menuentry -= 1;
                renderStartScreen(menuentry);
            }
            break;
        default:
            renderStartScreen(menuentry);
            state = 0;
            break;
    }
    
    return state;
}

int handleHighscoreScreen(uint32_t keybits) 
{
    int state = 2;
    int tmphighscores[NUMOFHIGHSCORES];
    // for(int i = 0; i < NUMOFHIGHSCORES; i++) {
    //     tmphighscores[i] = -1; 
    // }
    
    switch(keybits) {
        case ESC_BIT:
            state = 0;
            break;
        default:
            break;
    }

    if(xSemaphoreTake(highscores.lock, 0) == pdTRUE) {
        for(int i = 0; i < NUMOFHIGHSCORES; i++) {
            tmphighscores[i] = highscores.highscores[i];
        }
        xSemaphoreGive(highscores.lock);
    }

    //For testing:
    tmphighscores[0] = 69000; tmphighscores[1] = 42;
    renderHighscoreScreen(tmphighscores);
    
    return state;
}

int handleCheatsScreen(uint32_t keybits)
{
    int tmpstate = 3;
    static int menuentry = 1;
    struct gameparameters tmpparam = getParameters();
    char isactive = 0;

    switch(keybits) {
        case ESC_BIT:
            tmpstate = 0;
            break;
        case ENTER_BIT:
            if (menuentry == 1) {
                if (!tmpparam.inf_lives)
                    tmpparam.inf_lives = 1;
                else
                    tmpparam.inf_lives = 0;
            }
            else if(menuentry == 2)
                tmpparam.score = 0;
            if (menuentry == 3)
                tmpparam.level = 1;
            break;
        case DOWN_BIT:
            if(menuentry < 3)
                menuentry += 1;
            break;
        case UP_BIT:
            if(menuentry > 1)
                menuentry -= 1;
            break;
        case LEFT_BIT:
            if(menuentry == 2)
                tmpparam.score -= 100;
            else if(menuentry == 3 && tmpparam.level > 1)
                    tmpparam.level -= 1;
            break;
        case RIGHT_BIT:
            if(menuentry == 2)
                tmpparam.score += 100;
            else if(menuentry == 3 && tmpparam.level < 100)
                tmpparam.level += 1;
            break;
        default:
            renderCheatsScreen(menuentry, tmpparam);
            break;
    }

    renderCheatsScreen(menuentry, tmpparam);

    if (xSemaphoreTake(parameters.lock, 0) == pdTRUE) {
        parameters.inf_lives = tmpparam.inf_lives;
        parameters.lives = tmpparam.lives;
        parameters.score = tmpparam.score;
        parameters.level = tmpparam.level;
        xSemaphoreGive(parameters.lock);
    }
    
    return tmpstate;
}

int handleGameScreen(uint32_t keybits) 
{
    int state = 1;
    int tmppos = 0;
    char playershot = 0;
    static int menuentry = 1;
    int pausescreen = 0;

    switch(keybits){
        case ESC_BIT:
            if(pausescreen == 0) {
                pausescreen = 1;
                stopGame();
                renderPauseScreen(menuentry);
            }
            else {
                pausescreen = 0;
                resumeGame();
            }
            break;
        case SPACE_BIT:
            if(xSemaphoreTake(game.playershot.lock, 0) == pdTRUE) {
                playershot = game.playershot.active;
                xSemaphoreGive(game.playershot.lock);
            }

            if(playershot == 1)
                break;

            if(xSemaphoreTake(game.position.lock, 0) == pdTRUE) {
                tmppos = game.position.position;
                xSemaphoreGive(game.position.lock);
            }
            addPlayerProjectile(tmppos + 14, SCREEN_HEIGHT - FOOTERHEIGHT - PLAYERHEIGHT - PROJECTILESIZE, -1, Green, PROJECTILESIZE, PROJECTILESPEED, PROJECTILESPEED);
            break;
        case LEFT_BIT:
            if(xSemaphoreTake(game.position.lock, 0) == pdTRUE) {
                tmppos = game.position.position;
                if(tmppos >= LEFT_BORDER) {
                    game.position.position -= 8;
                    tmppos -= 8;
                }
                xSemaphoreGive(game.position.lock);
            }
            if(tmppos >= LEFT_BORDER)
                renderPlayer(-8);
            break;
        case RIGHT_BIT:
            if(xSemaphoreTake(game.position.lock, 0) == pdTRUE) {
                tmppos = game.position.position;
                if(tmppos <= SCREEN_WIDTH - RIGHT_BORDER) {
                    game.position.position += 8;
                    tmppos += 8;
                }
                xSemaphoreGive(game.position.lock);
            }
            if(tmppos <= SCREEN_WIDTH - RIGHT_BORDER)
                renderPlayer(8);
            break;
        case UP_BIT:
            if(menuentry > 1)
                menuentry -= 1;
            renderPauseScreen(menuentry);
            break;
        case DOWN_BIT:
            if(menuentry < 3)
                menuentry += 1;
            renderPauseScreen(menuentry);
            break;
        case ENTER_BIT:
            if(menuentry == 1) {
                pausescreen = 0;
                resumeGame();
            }
            if(menuentry == 2)
                state = 3;
            if(menuentry == 3){
                //stopGame();
                state = 0;
            }
            break;
        default:
            //resumeGame();
            break;
    }
    return state;
}

int handlePauseScreen(uint32_t keybits) 
{
    int state = 4;
    static int menuentry = 1;

    switch(keybits) {
        case ESC_BIT:
            state = 5;
            break;
        case ENTER_BIT:
            if(menuentry == 1) {//Back to Game
                state = 5;
            }
            else if(menuentry == 2) //Cheats
                state = 3;
            else if(menuentry == 3) { //startscreen
                state = 0;
                if(xSemaphoreTake(isgameactive.lock, 0) == pdTRUE) {
                    isgameactive.isit = 0;
                    xSemaphoreGive(isgameactive.lock);
                }
            }        
            break;
        case DOWN_BIT:
            if(menuentry < 3)
                menuentry += 1;
            renderPauseScreen(menuentry);
            break;
        case UP_BIT:
            if(menuentry > 1)
                menuentry -= 1;
            renderPauseScreen(menuentry);
            break;
        default:
            renderPauseScreen(1);
            break;
    }
    return state;
}

struct gameparameters getParameters()
{
    struct gameparameters tmpparam;

    if (xSemaphoreTake(parameters.lock, 0) == pdTRUE) {
        tmpparam.level = parameters.level;
        tmpparam.score = parameters.score;
        tmpparam.lives = parameters.lives;
        tmpparam.inf_lives = parameters.inf_lives;
        xSemaphoreGive(parameters.lock);
    }

    return tmpparam;
}

void switchState(int targetstate) 
{
    int tmpstate = 0;
    
    if (xSemaphoreTake(state.lock, 0) == pdTRUE) {
            tmpstate = state.state;
            xSemaphoreGive(state.lock);
        }

    if(targetstate == -1)
        exitGame();
    else if(targetstate != tmpstate) {
        if (xSemaphoreTake(state.lock, 0) == pdTRUE) {
            state.state = targetstate;
            xSemaphoreGive(state.lock);
        }
        switch(targetstate) {
            case 0:
                handleStartScreen(0x0000);
                break;
            case 1:
                handleGameScreen(0x0000);
                break;
            case 2:
                handleHighscoreScreen(0x0000);
                break;
            case 3:
                handleCheatsScreen(0x0000);
                break;
            case 4:
                handlePauseScreen(0x0000);
                break;
            case 5:
                xTimerStart(welcomeDuration, portMAX_DELAY);
                handleWelcomeScreen(0x0000);
                break;
            default:
                break;
        }
    }
    
    return;
}

int handleWelcomeScreen(uint32_t keybits)
{
    
    int score1 = 0;
    int hiscore = 0;
    int state = 5;

    if(xSemaphoreTake(highscores.lock, 0) == pdPASS) {
        hiscore = highscores.highscores[0];
        xSemaphoreGive(highscores.lock);
    }
    if(xSemaphoreTake(parameters.lock, 0) == pdPASS) {
        score1 = parameters.score;
        xSemaphoreGive(parameters.lock);
    }

    if (hiscore == -1)
        hiscore = 0;
    
    switch(keybits) {
        case ESC_BIT: //Go into Pause Screen
            //stop timer for vInitGame
            state = 4;
            break;
        default:
            renderWelcomeScreen(score1, hiscore);
            break;
    }
    return state;
}

void vInitGame(TimerHandle_t timer)
{
    
    clearScreen();
    
    if(xSemaphoreTake(game.position.lock, 0) == pdTRUE) {
        game.position.position = LEFT_BORDER;
        xSemaphoreGive(game.position.lock);
    }

    renderPlayer(0);
    
    int inf_lives = 0, level = 1, score1 = 0, hiscore = 0;
    if(xSemaphoreTake(parameters.lock, 0) == pdTRUE){
        inf_lives = parameters.inf_lives;
        level = parameters.level;
        score1 = parameters.score;
        xSemaphoreGive(parameters.lock);
    }
    if(xSemaphoreTake(highscores.lock, 0) == pdTRUE) {
        if(highscores.highscores[0] != -1)
            hiscore = highscores.highscores[0];
        xSemaphoreGive(highscores.lock);
    }
    renderHeader();
    renderFooter(3, level, inf_lives);

    startGame();
    
    //renderEnemies();

    //render player wall and sprite
    // on collision with player wall:
    //  - remove player
    //  - wait some time
    //  - reduce lives by one and start over
    //add enemies and their walls to enemy queue
    //enemies are rendered by gametick renderer
    // on collision with enemy wall:
    //  - destroy wall
    //  - play sound
    //  - remove enemy from queue
    //  - increase gametick (make faster)
    //  - Do explosion animation as extra later (not needed for working game)
    // on collision with end wall:
    //  - Play sound
    //  - 

    //player only needs to be updated on right or left or on player collision (removed)
    //enemies only need to be updated on gametick or player collision
    //score only needs to be updated on enemy collision
    //lives only needs to be updated on player collision or enemy intrusion
    //hi-score only needs to be updated if lives==0 and score > any other hiscore
    //there is a task running that adds enemy projectiles randomly to the projectile queue
    switchState(1);
    // if(xSemaphoreTake(state.lock, 0) == pdTRUE) {
    //     state.state = 1;
    //     xSemaphoreGive(state.lock);
    // }
}

void resumeGame()
{
    clearScreen();
    renderPlayer(0);
    renderHeader();

    int inf_lives = 0, level = 1, lives = 3;
    if(xSemaphoreTake(parameters.lock, 0) == pdTRUE){
        inf_lives = parameters.inf_lives;
        level = parameters.level;
        lives = parameters.lives;
        xSemaphoreGive(parameters.lock);
    }
    renderFooter(lives, level, inf_lives);

    startGame();

    if(xSemaphoreTake(state.lock, 0) == pdTRUE) {
        state.state = 1;
        //queueRedCircle();
        xSemaphoreGive(state.lock);
    }
}

void onPlayerHit(void* args){
    queueRedCircle();
}


//###### TASKS ######//
void vEmitKeys()
{
    unsigned char last_keystate[NUMOFKEYS] = { 0 };
    unsigned char cur_keystate[NUMOFKEYS] = { 0 };
    uint32_t emit_bits[NUMOFKEYS] = {
        ENTER_BIT,
        ESC_BIT,
        SPACE_BIT,
        DOWN_BIT,
        UP_BIT,
        LEFT_BIT,
        RIGHT_BIT
    };
    uint32_t keybits = 0x0000;

    int debouncelim = 4;
    int debouncecounter[2] = {0, 0};
    
    while(1) {
        //Fetch newest key lookup table and load it in from its queue
        keybits = 0x0000;
        tumEventFetchEvents(FETCH_EVENT_NONBLOCK | FETCH_EVENT_NO_GL_CHECK);
        if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
            xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
            
            cur_keystate[0] =  buttons.buttons[ENTER];
            cur_keystate[1] =  buttons.buttons[ESC];
            cur_keystate[2] =  buttons.buttons[SPACE];
            cur_keystate[3] =  buttons.buttons[DOWN];
            cur_keystate[4] =  buttons.buttons[UP];
            cur_keystate[5] =  buttons.buttons[LEFT];
            cur_keystate[6] =  buttons.buttons[RIGHT];
            xSemaphoreGive(buttons.lock);
        }

        for(int i = 0; i < NUMOFKEYS - 2; i++) {
                if(last_keystate[i] == 1 && cur_keystate[i] == 0) {
                    last_keystate[i] = 0;
                    keybits |= emit_bits[i];
                }
                else if(last_keystate[i] == 0 && cur_keystate[i] == 1) {
                    last_keystate[i] = 1;
                }
        }

        if(cur_keystate[5] == 1) {
            debouncecounter[0] += 1;
            if(debouncecounter[0] == debouncelim) {
                debouncecounter[0] = 0;
                keybits |= emit_bits[5];
            }
        }
        if(cur_keystate[6] == 1) {
            debouncecounter[1] += 1;
            if(debouncecounter[1] == debouncelim) {
                debouncecounter[1] = 0;
                keybits |= emit_bits[6];
            }
        }

        if(keybits != 0x0000)
            xTaskNotify(vOnEnterHandle, keybits, eSetValueWithOverwrite);
            //Above line does not work with eSetBits!

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vOnEnter()
{
    uint32_t ulNotifiedValue;
    uint32_t xResult;
    int curstate;

    while(1) {
        xResult = xTaskNotifyWait(pdFALSE, pdTRUE, &ulNotifiedValue, portMAX_DELAY);
        
        if(xResult == pdPASS) {
            
            if (xSemaphoreTake(state.lock, 0) == pdTRUE) {
                curstate = state.state;
                xSemaphoreGive(state.lock);
            }
            
            switch(curstate) {
                case 0:
                    switchState(handleStartScreen(ulNotifiedValue));
                    break;
                case 1:
                    switchState(handleGameScreen(ulNotifiedValue));
                    break;
                case 2:
                    switchState(handleHighscoreScreen(ulNotifiedValue));
                    break;
                case 3:
                    switchState(handleCheatsScreen(ulNotifiedValue));
                    break;
                case 4:
                    switchState(handlePauseScreen(ulNotifiedValue));
                    break;
                case 5:
                    switchState(handleWelcomeScreen(ulNotifiedValue));
                    break;
                default:
                    break;
            }
        }
    }
}