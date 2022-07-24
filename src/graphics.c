#include "graphics.h"
#include "FreeRTOS.h"
#include "task.h"
#include "TUM_Draw.h"
#include "TUM_Font.h"
#include "objects.h"
#include "TUM_Ball.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


static TaskHandle_t vRenderHandle = NULL;
static TaskHandle_t vBallOnCollisionHandle = NULL;
static TaskHandle_t vGameTickRenderHandle = NULL;
static TaskHandle_t vProjectileHandlerHandle = NULL;

extern struct game game;
extern struct playerhitbox playherhitbox;
extern gameparammutex parameters;
extern hs highscores;

extern int enemyid[ENEMYROW][ENEMYCOL][2];
extern int projectileid[MAXPROJECTILES][2];

int initGraphics(char* bin_folder_path)
{
    
    srand(time(NULL));
    //back wall;
    wall_t* backwall = createWall(0, UPPER_BORDER + CEILING_THICKNESS - 50, SCREEN_WIDTH, 50 - PROJECTILESIZE, -1, Black, &wallOnCollision, NULL);
    wall_t* lowerwall = createWall(0, LOWER_BORDER + 50, SCREEN_WIDTH, 50, -1, Black, &wallOnCollision, NULL);
    
    for(int i = 0; i < ENEMYROW; i++) {
        for(int j = 0; j < ENEMYCOL; j++) {
            enemyid[i][j][0] = i;
            enemyid[i][j][1] = j;
        }
    }

    for(int i = 0; i < MAXPROJECTILES; i++) {
        projectileid[i][0] = i;
    }

    if(xSemaphoreTake(game.gamespeed.lock, 0) == pdTRUE) {
        game.gamespeed.gamespeed = 1;
        xSemaphoreGive(game.gamespeed.lock);
    }

    
    if(xSemaphoreTake(playerhitbox.lock, 0) == pdTRUE) {
        playerhitbox.box = createWall(LEFT_BORDER, SCREEN_HEIGHT - FOOTERHEIGHT - PLAYERHEIGHT + 10, 30, 15, -1, White, NULL, NULL);
        xSemaphoreGive(playerhitbox.lock);
    }
    
    
    if(xSemaphoreTake(game.projectileptrs.lock, 0) == pdTRUE) {
        for(int i = 0; i < MAXPROJECTILES; i++) {
            game.projectileptrs.list[i] = createBall(INACTIVE, INACTIVE, Red, PROJECTILESIZE, 
                        PROJECTILESPEED, &ballOnCollision, &projectileid[i]);
            setBallSpeed(game.projectileptrs.list[i], 0, 0, 
                    PROJECTILESPEED, SET_BALL_SPEED_Y);
        }
        xSemaphoreGive(game.projectileptrs.lock);
    }

    
    if(xSemaphoreTake(game.enemyptrs.lock, 0) == pdTRUE) {
        //int id[2] = {0, 0};
        for(int i = 0; i < ENEMYROW; i++) {
            for(int j = 0; j < ENEMYCOL; j++) {
                game.enemyptrs.enemies[i][j] = NULL;
                //int id[2] = {0, 0};
                wall_t* tmpwall = NULL;
                if(i == 0)
                    tmpwall = createWall(LEFT_BORDER - ENEMYSIZE / 2 + 11 + j * ENEMYHSPACING, UPPER_BORDER + ENEMYSTARTY + 10 + i * ENEMYVSPACING, ENEMYSIZE - 5, ENEMYSIZE, -1, Yellow, &enemyOnCollision, &enemyid[i][j]);
                else if(i <= 2)
                    tmpwall = createWall(LEFT_BORDER - ENEMYSIZE / 2 + 4 + j * ENEMYHSPACING, UPPER_BORDER + ENEMYSTARTY + 10 + i * ENEMYVSPACING, ENEMYSIZE + 7, ENEMYSIZE, -1, Yellow, &enemyOnCollision, &enemyid[i][j]);
                else
                    tmpwall = createWall(LEFT_BORDER - ENEMYSIZE / 2 + 5 + j * ENEMYHSPACING, UPPER_BORDER + ENEMYSTARTY + 10 + i * ENEMYVSPACING, ENEMYSIZE + 9, ENEMYSIZE, -1, Yellow, &enemyOnCollision, &enemyid[i][j]);
                game.enemyptrs.enemies[i][j] = tmpwall;
            }
        }
        xSemaphoreGive(game.enemyptrs.lock);
    }

    
    if(xSemaphoreTake(game.projectilestates.lock, 0) == pdTRUE) {
        for(int i = 0; i < MAXPROJECTILES; i++) {
            game.projectilestates.states[i] = 0;
        }
        xSemaphoreGive(game.projectilestates.lock);
    }

    
    if(xSemaphoreTake(game.enemystates.lock, 0) == pdTRUE) {
        for(int i = 0; i < ENEMYROW; i++) {
            for(int j = 0; j < ENEMYCOL; j++) {
                game.enemystates.states[i][j] = 1;
            }
        }
        xSemaphoreGive(game.enemystates.lock);
    }

    if(xSemaphoreTake(game.playershot.lock, 0) == pdTRUE) {
        game.playershot.shot = createBall(INACTIVE, INACTIVE, Green, PROJECTILESIZE, PROJECTILESPEED, &onPlayerProjectileCollision, NULL);
        xSemaphoreGive(game.playershot.lock);
    }
    
    int init_fault = tumDrawInit(bin_folder_path) || tumFontInit(bin_folder_path) ||
            !game.projectileptrs.lock || !game.enemyptrs.lock || !game.enemystates.lock ||
            !game.projectilestates.lock ||
            (xTaskCreate(vRender, "vRender", mainGENERIC_STACK_SIZE * 16,
                    NULL, (configMAX_PRIORITIES - 1), &vRenderHandle) != pdPASS) ||
            (xTaskCreate(vGameTickRender, "vGameTickRender", mainGENERIC_STACK_SIZE * 16,
                    NULL, (configMAX_PRIORITIES - 2), &vGameTickRenderHandle) != pdPASS) ||
            (xTaskCreate(vProjectileHandler, "vProjectileHandler", mainGENERIC_STACK_SIZE * 4,
                     NULL, (configMAX_PRIORITIES - 1), &vProjectileHandlerHandle) != pdPASS) ||
            (xTaskCreate(vRandomProjectiles, "vRandomProjectiles", mainGENERIC_STACK_SIZE * 4,
                     NULL, (configMAX_PRIORITIES - 1), &vRandomProjectilesHandle) != pdPASS);

    stopGame(); //Suspends gametick renderer for now

    
    if (init_fault) {
        printf("Failed to intialize graphics!\n");
        return -1;
    }
    else {
        printf("Graphics initialized.\n");
    }

    tumFontLoadFont("space_invaders.ttf", 30);
    tumFontSelectFontFromName("space_invaders.ttf"); 	
    logo_image = 
    tumDrawLoadImage("../resources/images/320px-Space_invaders_logo.png");
    exit_image = 
    tumDrawLoadImage("../resources/images/191px-exit.png");
    startbutton_active_image = 
    tumDrawLoadImage("../resources/images/210px-button_start_game_active.png");
    startbutton_inactive_image = 
    tumDrawLoadImage("../resources/images/210px-button_start_game_inactive.png");
    hsbutton_active_image = 
    tumDrawLoadImage("../resources/images/210px-button_highscores_active.png");
    hsbutton_inactive_image = 
    tumDrawLoadImage("../resources/images/210px-button_highscores_inactive.png");
    cheatsbutton_active_image = 
    tumDrawLoadImage("../resources/images/210px-button_cheats_active.png");
    cheatsbutton_inactive_image = 
    tumDrawLoadImage("../resources/images/210px-button_cheats_inactive.png");
    arrow_keys_image = 
    tumDrawLoadImage("../resources/images/120px-arrow_keys.png");
    spacebar_image = 
    tumDrawLoadImage("../resources/images/120px-spacebar.png");
    enter_key_image = 
    tumDrawLoadImage("../resources/images/120px-enter_key.png");
    cheats_header_image = 
    tumDrawLoadImage("../resources/images/269px-header_cheats.png");
    cheats1_active_image =
    tumDrawLoadImage("../resources/images/208px_button_infinite_lives_active.png");
    cheats2_active_image =
    tumDrawLoadImage("../resources/images/208px-button_starting_score_active.png");
    cheats3_active_image =
    tumDrawLoadImage("../resources/images/208px-button_starting_level_active.png");
    cheats1_inactive_image = 
    tumDrawLoadImage("../resources/images/208px_button_infinite_lives_inactive.png");
    cheats2_inactive_image =
    tumDrawLoadImage("../resources/images/208px-button_starting_score_inactive.png");
    cheats3_inactive_image =
    tumDrawLoadImage("../resources/images/208px-button_starting_level_inactive.png");
    highscores_header_image = 
    tumDrawLoadImage("../resources/images/269px-header_highscores.png");

    spritesheet_image = 
    tumDrawLoadImage("../resources/images/spritesheet_scaled.png");
    spritesheet = tumDrawLoadSpritesheet(spritesheet_image, 9, 3);


    return 0;
}

void startGame() {
    vTaskResume(vGameTickRenderHandle);
    vTaskResume(vProjectileHandlerHandle);
    vTaskResume(vRandomProjectilesHandle);
}

void stopGame() {
    vTaskSuspend(vGameTickRenderHandle);
    vTaskSuspend(vProjectileHandlerHandle);
    vTaskSuspend(vRandomProjectilesHandle);
}

void vRender()
{
    int i = 0;
    ball_t* tmpballs[MAXPROJECTILES] = { NULL };
    ball_t* playershot = NULL;
    
    tumDrawBindThread();
    
    while(1) {
        //ball_t* plist[MAXPROJECTILES] = { NULL };
        //copy projectile list into task memory and leave semaphore
        // if(xSemaphoreTake(game.projectileptrs.lock, 0) == pdTRUE) {
        //     //If projectiles are present, remove their last rendered position
        //     i = 0;
        //     while((game.projectileptrs.list[i] == NULL) && (i < MAXPROJECTILES))
        //         i++;
        //     if(i < MAXPROJECTILES)
        //         //tumDrawFilledBox(0, UPPER_BORDER + CEILING_THICKNESS, SCREEN_WIDTH, SCREEN_HEIGHT - FOOTERHEIGHT - UPPER_BORDER - PLAYERHEIGHT - GROUND_THICKNESS - CEILING_THICKNESS, Black);
            
        //     for(i = 0; i < MAXPROJECTILES; i++) {
        //         if(game.projectileptrs.list[i] != NULL){
        //             //If a ball in the ball renderqueue collided, remove it
        //             if(checkBallCollisions(game.projectileptrs.list[i], NULL, NULL)) {
        //                 renderProjectile(game.projectileptrs.list[i], Black);
        //                 game.projectileptrs.list[i] = NULL;
        //                 continue;
        //             }
        //             renderProjectile(game.projectileptrs.list[i], Black);
        //             updateBallPosition(game.projectileptrs.list[i], frametime);
        //             renderProjectile(game.projectileptrs.list[i], Green);
        //         }
        //     }
        //     xSemaphoreGive(game.projectileptrs.lock);
        // }

        if(xSemaphoreTake(game.projectileptrs.lock, 0) == pdTRUE) {
            for(int i = 0; i < MAXPROJECTILES; i++)
                tmpballs[i] = game.projectileptrs.list[i];
            xSemaphoreGive(game.projectileptrs.lock);
        }

        if(xSemaphoreTake(game.playershot.lock, 0) == pdTRUE) {
            playershot = game.playershot.shot;
            xSemaphoreGive(game.playershot.lock);
        }

        for(int i = 0; i < MAXPROJECTILES; i++) {
            if(tmpballs[i]->y < SCREEN_HEIGHT - FOOTERHEIGHT - PLAYERHEIGHT - GROUND_THICKNESS - 20)
                checkBallCollisions(tmpballs[i], NULL, NULL);
        }
        checkBallCollisions(playershot, NULL, NULL);

        handleProjectiles();
        //renderEnemies();

        tumDrawUpdateScreen();
        vTaskDelay(pdMS_TO_TICKS(FRAMETIME));
    }
}

void vGameTickRender()
{
    int gamespeed = 0;
    char animationstate = 0;

    char enemystates[ENEMYROW][ENEMYCOL] = { 0 };

    signed char direction = 1;
    signed char wallsetbit = SET_WALL_X;

    while(1) {
        wallsetbit = SET_WALL_X;
        
        if(xSemaphoreTake(game.gamespeed.lock, 0) == pdTRUE) {
            gamespeed = game.gamespeed.gamespeed;
            xSemaphoreGive(game.gamespeed.lock);
        }

        if(xSemaphoreTake(game.enemystates.lock, 0) == pdTRUE) {
            for(int i = 0; i < ENEMYROW; i++) {
                for(int j = 0; j < ENEMYCOL; j++) {
                    enemystates[i][j] = game.enemystates.states[i][j];
                }
            }
            xSemaphoreGive(game.enemystates.lock);
        }

        if(xSemaphoreTake(game.enemyptrs.lock, 0) == pdTRUE) {
            //Find out if an enemy has a collision with left or right border
            for(int i = 0; i < ENEMYROW; i++) {
                for(int j = 0; j < ENEMYCOL; j++) {
                    if(enemystates[i][j] == 1) {
                        if(game.enemyptrs.enemies[i][j]->x1 + game.enemyptrs.enemies[i][j]->w > SCREEN_WIDTH) {
                            direction = -1;
                            wallsetbit = SET_WALL_AXES;
                        }
                        else if(game.enemyptrs.enemies[i][j]->x1 < 0) {
                            direction = 1;
                            wallsetbit = SET_WALL_AXES;
                        }
                    }
                }
            }
            
            for(int i = 0; i < ENEMYROW; i++) {
                for(int j = 0; j < ENEMYCOL; j++) {
                    if(enemystates[i][j] == 1) {
                        //draw over last position
                        tumDrawFilledBox(game.enemyptrs.enemies[i][j]->x1, game.enemyptrs.enemies[i][j]->y1, game.enemyptrs.enemies[i][j]->w, game.enemyptrs.enemies[i][j]->h, Black);
                        setWallProperty(game.enemyptrs.enemies[i][j], game.enemyptrs.enemies[i][j]->x1 + direction * 10, game.enemyptrs.enemies[i][j]->y1 + 40, 0, 0, wallsetbit);
                    }
                    //setWallProperty(game.enemyptrs.enemies[i][j], game.enemyptrs.enemies[i][j]->x1 + direction * 10, game.enemyptrs.enemies[i][j]->y1 + 40, 0, 0, wallsetbit);
                }
            }
            xSemaphoreGive(game.enemyptrs.lock);
        }
        
        
        renderEnemies(animationstate);
        // renderEnemies(animationstate);

        if(animationstate == 0) 
            animationstate = 1;
        else 
            animationstate = 0;

        vTaskDelay((TickType_t)(BASEGAMETICK) - gamespeed);
    }
}

void vProjectileHandler()
{
   

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000/10));
    }
}

void vBallOnCollision()
{
    uint32_t notification = 0x0000;
    
    while(1) {
        xTaskNotifyWait(pdFALSE, pdTRUE, &notification, portMAX_DELAY);
        //queueRedCircle(); For checking if notification worked
        // game.projectileptrs.list[0] = NULL;

        // if(xSemaphoreTake(game.projectileptrs.lock, 0) == pdTRUE) {
        //     game.projectileptrs.list[0] = NULL;
        //     xSemaphoreGive(game.projectileptrs.lock);
        // }
    }
}

void vRandomProjectiles()
{
    int delay = 0;
    int x = 0;
    int gamespeed = 0;
    int chosenoneid[2] = { 0, 0 };
    wall_t* thechosenone = NULL;
    int numoflowestenemies = 0;
    char rowfound = 0;
    int i = 0, j = 0;

    while(1) {

        
        if(xSemaphoreTake(game.gamespeed.lock, 0) == pdTRUE) {
            gamespeed = game.gamespeed.gamespeed;
            xSemaphoreGive(game.gamespeed.lock);
        }
        
        //Count active enemies of lowest row
        // if(xSemaphoreTake(game.enemystates.lock, 0) == pdTRUE) {
        //     i = 0; j = 0; numoflowestenemies = 0; rowfound = 0;
        //     for(i = ENEMYROW - 1; i >= 0; i--) {
        //         if(rowfound == 1)
        //             break;
        //         for(j = ENEMYCOL - 1; j >= 0; j--) {
        //             if(game.enemystates.states[i][j] == 1) {
        //                 rowfound = 1;
        //                 break;
        //             }
        //         }
        //     }
        //     for(j = 0; j < ENEMYCOL; j++) {
        //         if(game.enemystates.states[i][j] == 1)
        //             numoflowestenemies++;
        //     }
        //     xSemaphoreGive(game.enemystates.lock);
        // }


            x = rand() % (SCREEN_WIDTH - LEFT_BORDER - RIGHT_BORDER);
            addProjectile(x, SCREEN_HEIGHT - 250, 1, Red, PROJECTILESIZE, PROJECTILESPEED, PROJECTILESPEED);

            int delay = rand() % (gamespeed + 1) + 1000;
            vTaskDelay(pdMS_TO_TICKS(delay));
        }
}

void queueRedCircle()
{
    tumDrawCircle(320, 240, 100, Red);
}

void renderStartScreen(int selectedmenuentry)
{

    tumDrawClear(Black);   
    tumDrawLoadedImage(logo_image, MIDX - 160, 20);
    tumDrawLoadedImage(exit_image, MIDX - 95, SCREEN_HEIGHT - 40);
    tumDrawLoadedImage(arrow_keys_image, SCREEN_WIDTH - 140, SCREEN_HEIGHT - 200);
    tumDrawLoadedImage(spacebar_image, SCREEN_WIDTH - 140, SCREEN_HEIGHT - 50);
    tumDrawLoadedImage(enter_key_image, SCREEN_WIDTH - 140, SCREEN_HEIGHT - 100);
    
    if(selectedmenuentry == 1) {
        tumDrawLoadedImage(startbutton_active_image, MIDX - 105, MIDY);
        tumDrawLoadedImage(hsbutton_inactive_image, MIDX - 105, MIDY + 80);
        tumDrawLoadedImage(cheatsbutton_inactive_image, MIDX - 105, MIDY + 160);
    }
    else if(selectedmenuentry == 2) {
        tumDrawLoadedImage(startbutton_inactive_image, MIDX - 105, MIDY);
        tumDrawLoadedImage(hsbutton_active_image, MIDX - 105, MIDY + 80);
        tumDrawLoadedImage(cheatsbutton_inactive_image, MIDX - 105, MIDY + 160);
    }
    else if(selectedmenuentry == 3) {
        tumDrawLoadedImage(startbutton_inactive_image, MIDX - 105, MIDY);
        tumDrawLoadedImage(hsbutton_inactive_image, MIDX - 105, MIDY + 80);
        tumDrawLoadedImage(cheatsbutton_active_image, MIDX - 105, MIDY + 160);
    }
    else if(selectedmenuentry == 4) {
        tumDrawClear(Black);
        queueRedCircle();
    }

}

void renderHeader()
{
    
    int score1 = 0;
    int hiscore = 0;
    
    if(xSemaphoreTake(parameters.lock, 0) == pdTRUE) {
        score1 = parameters.score;
        xSemaphoreGive(parameters.lock);
    }


    if(xSemaphoreTake(highscores.lock, 0) == pdTRUE) {
        hiscore = highscores.highscores[0];
        xSemaphoreGive(highscores.lock);
    }

    char gap = 10;

    int w = 0, h = 0, scorew = 0;
    char strscore1[] = "00000";
    char strhiscore[] = "00000";

    tumFontSetSize(20);
    tumDrawFilledBox(0, 0, SCREEN_WIDTH, 100, Black);

    tumGetTextSize("SCORE<1>", &scorew, &h);
    tumDrawText("SCORE<1>", gap, gap, White);
    tumGetTextSize("HI-SCORE", &w, &h);
    tumDrawText("HI-SCORE", MIDX - w / 2, gap, White);
    tumGetTextSize("SCORE<2>", &w, &h);
    tumDrawText("SCORE<2>", SCREEN_WIDTH - w - gap, gap, White);

    sprintf(strscore1, "%.4i", score1);
    tumGetTextSize(strscore1, &w, &h);
    tumDrawText(strscore1, gap + (scorew - w) / 2, h + gap, White);
    sprintf(strhiscore, "%.4i", hiscore);
    tumGetTextSize(strhiscore, &w, &h);
    tumDrawText(strhiscore, MIDX - w / 2, h + gap, White);
    tumDrawText("0000", SCREEN_WIDTH - gap - (scorew - w) / 2 - w, h + gap, White);

    tumDrawFilledBox(0, UPPER_BORDER, SCREEN_WIDTH, CEILING_THICKNESS, Grey);
}

void renderWelcomeScreen(int score1, int hiscore)
{
    int w = 0, h = 0, gap = 40, offset = 25;
    
    tumFontSetSize(20);
    tumDrawClear(Black);

    renderHeader(score1, hiscore);
    tumGetTextSize("Play", &w, &h);
    tumDrawText("Play", MIDX - w / 2, MIDY - 4 * h, White);
    tumGetTextSize("Space       Invaders", &w, &h);
    tumDrawText("Space      Invaders", MIDX - w / 2, MIDY - 2 * h, White);
    tumGetTextSize("*SCORE    ADVANCE    TABLE*", &w, &h);
    tumDrawText("*SCORE    ADVANCE    TABLE*", MIDX - w / 2, MIDY + h, White);

    tumDrawSprite(spritesheet, 3, 2, MIDX - 100, MIDY + h + gap);
    tumDrawSprite(spritesheet, 3, 0, MIDX - 100, MIDY + h + gap + 50);
    tumDrawSprite(spritesheet, 1, 0, MIDX - 100, MIDY + h + gap + 100);
    tumDrawSprite(spritesheet, 6, 0, MIDX - 100, MIDY + h + gap + 150);

    tumDrawText("=? MYSTERY", MIDX - gap, MIDY + h + gap + offset, White);
    tumDrawText("=30 POINTS", MIDX - gap, MIDY + h + gap + 50 + offset, White);
    tumDrawText("=20 POINTS", MIDX - gap, MIDY + h + gap + 100 + offset, White);
    tumDrawText("=10 POINTS", MIDX - gap, MIDY + h + gap + 150 + offset, White);

    tumGetTextSize("LEVEL **", &w, &h);
    tumDrawText("LEVEL **", SCREEN_WIDTH - w - 10, SCREEN_HEIGHT - h - 20, White);

}

void renderCheatsScreen(int menuentry, struct gameparameters param)
{
    char score[10] = "0";
    char level[10] = "1";
    
    tumDrawClear(Black);
    tumDrawLoadedImage(cheats_header_image, MIDX - 135, 20);
    if(menuentry == 1) {
        tumDrawLoadedImage(cheats1_active_image, MIDX - 208, MIDY - 80);
        tumDrawLoadedImage(cheats2_inactive_image, MIDX - 208, MIDY);
        tumDrawLoadedImage(cheats3_inactive_image, MIDX - 208, MIDY + 80);
    
    }
    if(menuentry == 2) {
        tumDrawLoadedImage(cheats1_inactive_image, MIDX - 208, MIDY - 80);
        tumDrawLoadedImage(cheats2_active_image, MIDX - 208, MIDY);
        tumDrawLoadedImage(cheats3_inactive_image, MIDX - 208, MIDY + 80);
    }
    if(menuentry == 3) {
        tumDrawLoadedImage(cheats1_inactive_image, MIDX - 208, MIDY - 80);
        tumDrawLoadedImage(cheats2_inactive_image, MIDX - 208, MIDY);
        tumDrawLoadedImage(cheats3_active_image, MIDX - 208, MIDY + 80);
    }

    tumDrawFilledBox(MIDX + 20, MIDY - 80, 200, 80, Black);
    tumDrawFilledBox(MIDX + 20, MIDY + 10, 200, 80, Black);
    tumDrawFilledBox(MIDX + 20, MIDY + 60, 200, 80, Black);

    tumFontSetSize(30);
    if(param.inf_lives == 1)
        tumDrawText("Yes", MIDX + 20, MIDY - 60, White);
    else
        tumDrawText("No", MIDX + 20, MIDY - 60, White);
    sprintf(score, "%d", param.score);
    tumDrawText(score, MIDX + 20, MIDY + 20, White);
    sprintf(level, "%d", param.level);
    tumDrawText(level, MIDX + 20, MIDY + 100, White);
    tumFontSetSize(DEFAULT_FONT_SIZE);
}

void renderHighscoreScreen(int* highscores) 
{
    char tmpscorestr[10] = "0"; 
    int tmpscore = -1;
    
    tumDrawClear(Black);
    tumDrawLoadedImage(highscores_header_image, MIDX - 135 , 20);
    
    tumFontSetSize(30);
    for(int i = 0; i < NUMOFHIGHSCORES; i++) {
        tmpscore = highscores[i];
        //Scores are only printed if they are valid; Unused score slots are set to
        //-1 and invalid;
        if(tmpscore != -1) {
            sprintf(tmpscorestr, "%d", tmpscore);
            //tumDrawFilledBox(MIDX - 100, (MIDY - 70) + i * 50, 200, 50, Black);
            tumDrawText(tmpscorestr, MIDX - 100, (MIDY - 50) + i * 50, White);
        }

    }
    tumFontSetSize(DEFAULT_FONT_SIZE);
}

void renderPauseScreen(int menuentry)
{
    int menuwidth = 300;
    int menuheight = 500;
    int w = 0; int h = 0;

    tumDrawFilledBox(MIDX - menuwidth / 2, MIDY - menuheight / 2, menuwidth, menuheight, Grey);

    switch(menuentry) {
        case 1:
            tumGetTextSize("Back to Game", &w, &h);
            tumDrawText("Back To Game", MIDX - w / 2, MIDY - 2 * h, Green);
            tumGetTextSize("Cheats", &w, &h);
            tumDrawText("Cheats", MIDX - w / 2, MIDY, White);
            tumGetTextSize("Home", &w, &h);
            tumDrawText("Home", MIDX - w / 2, MIDY + 2 * h, White);
            break;
        case 2:
            tumGetTextSize("Back to Game", &w, &h);
            tumDrawText("Back To Game", MIDX - w / 2, MIDY - 2 * h, White);
            tumGetTextSize("Cheats", &w, &h);
            tumDrawText("Cheats", MIDX - w / 2, MIDY, Green);
            tumGetTextSize("Home", &w, &h);
            tumDrawText("Home", MIDX - w / 2, MIDY + 2 * h, White);
            break;
        case 3:
            tumGetTextSize("Cheats", &w, &h);
            tumDrawText("Cheats", MIDX - w / 2, MIDY, White);
            tumGetTextSize("Home", &w, &h);
            tumDrawText("Home", MIDX - w / 2, MIDY + 2 * h, Green);
            tumGetTextSize("Back to Game", &w, &h);
            tumDrawText("Back To Game", MIDX - w / 2, MIDY - 2 * h, White);
            break;
        default:
            break;
    }
}

void clearImages()
{

}

void renderPlayer(int offset)
{
    wall_t* hitbox = NULL;

    if(xSemaphoreTake(playerhitbox.lock, 0) == pdTRUE) {
        hitbox = playerhitbox.box;
        xSemaphoreGive(playerhitbox.lock);
    }
    
    //Update playerhitbox position
    setWallProperty(hitbox, hitbox->x1 + offset,
        hitbox->y1, hitbox->w, hitbox->h, 'a' );
    
    //Draw Playerhitbox; Only for debugging
    // tumDrawFilledBox(playerhitbox->x1, playerhitbox->y1, 
    //     playerhitbox->w, playerhitbox->h, playerhitbox->colour);
    //Draw player sprite
    tumDrawFilledBox(0, SCREEN_HEIGHT - FOOTERHEIGHT- GROUND_THICKNESS - PLAYERHEIGHT - 10, SCREEN_WIDTH, PLAYERHEIGHT, Black);
    tumDrawSprite(spritesheet, 4, 2, hitbox->x1 - 14, hitbox->y1 - 34);

}

void renderEnemies(char animationstate)
{
    //static char animationstate = 0;
    wall_t* tmpenemies[ENEMYROW][ENEMYCOL] = { NULL };
    char tmpstates[ENEMYCOL][ENEMYROW] = { 1 };

    
    if(xSemaphoreTake(game.enemystates.lock, 0) == pdTRUE) {
        for(int i = 0; i < ENEMYROW; i++) {
            for(int j = 0; j < ENEMYCOL; j++)
                tmpstates[i][j] = game.enemystates.states[i][j];
        }
        xSemaphoreGive(game.enemystates.lock);
    }

    if(xSemaphoreTake(game.enemyptrs.lock, 0) == pdTRUE) {
        for(int i = 0; i < ENEMYROW; i++) {
            for(int j = 0; j < ENEMYCOL; j++) {
                if(tmpstates[i][j] == 1) {
                    // For Debugging change color to something other than black:
                    tumDrawFilledBox(game.enemyptrs.enemies[i][j]->x1, game.enemyptrs.enemies[i][j]->y1, game.enemyptrs.enemies[i][j]->w, game.enemyptrs.enemies[i][j]->h, Black);
                    if(i == 0)
                        tumDrawSprite(spritesheet, 3 + animationstate, 0, game.enemyptrs.enemies[i][j]->x1 - 10, game.enemyptrs.enemies[i][j]->y1 - 10);
                    else if(i <= 2)
                        tumDrawSprite(spritesheet, 1 + animationstate, 0, game.enemyptrs.enemies[i][j]->x1 - 4,  game.enemyptrs.enemies[i][j]->y1 - 10);
                    else
                        tumDrawSprite(spritesheet, 5 + animationstate, 0, game.enemyptrs.enemies[i][j]->x1 - 4,  game.enemyptrs.enemies[i][j]->y1 - 10);
                }
            }
        }
        xSemaphoreGive(game.enemyptrs.lock);
    }

    // for(int i = 0; i < ENEMYROW; i++) {
    //     for(int j = 0; j < ENEMYCOL; j++) {
    //         if(tmpstates[i][j] == 1) {
    //             // For Debugging:
    //             tumDrawFilledBox(tmpenemies[i][j]->x1, tmpenemies[i][j]->y1, tmpenemies[i][j]->w, tmpenemies[i][j]->h, Black);
    //             if(i == 0)
    //                 tumDrawSprite(spritesheet, 3 + animationstate, 0, tmpenemies[i][j]->x1 - 10, tmpenemies[i][j]->y1);
    //             else if(i <= 2)
    //                 tumDrawSprite(spritesheet, 1 + animationstate, 0, LEFT_BORDER - ENEMYSIZE / 2 + j * ENEMYHSPACING , UPPER_BORDER + 100 + i * ENEMYVSPACING);
    //             else
    //                 tumDrawSprite(spritesheet, 5 + animationstate, 0, LEFT_BORDER - ENEMYSIZE / 2 + j * ENEMYHSPACING , UPPER_BORDER + 100 + i * ENEMYVSPACING);
    //         }
    //     }
    // }

    // if(animationstate == 0)
    //     animationstate = 1;
    // else
    //     animationstate = 0;
}

void ballCollisionCallbackInContext()
{
    // game.projectileptrs.list[0] = NULL;
    // tumDrawFilledBox(0, 150, SCREEN_WIDTH, SCREEN_HEIGHT - 270, Black);
    //queueRedCircle();
}

void onPlayerProjectileCollision()
{
    ball_t* playershot = NULL;

    if(xSemaphoreTake(game.playershot.lock, 0) == pdTRUE) {
        playershot = game.playershot.shot;
        game.playershot.active = 0;
        setBallSpeed(playershot, 0, 0, PROJECTILESPEED, SET_BALL_SPEED_ALL);
        tumDrawCircle(playershot->x, playershot->y,
                    playershot->radius, Black);
        xSemaphoreGive(game.playershot.lock);
    }
}

void wallOnCollision()
{
   //queueRedCircle();
}
void playerOnCollision()
{

}

void enemyOnCollision(int* id)
{
    wall_t* tmpwall = NULL;
    float_t speedincrease = BASEGAMETICK/(ENEMYCOL*ENEMYROW);

    if(xSemaphoreTake(parameters.lock, 0) == pdTRUE) {
        if(id[0] >= 3)
            parameters.score += EASYENEMYSCORE;
        else if (id[0] >= 1 && id[0] <= 2)
            parameters.score += MEDIUMENEMYSCORE;
        else
            parameters.score += HARDENEMYSCORE;
        xSemaphoreGive(parameters.lock);
    }
    

    if(xSemaphoreTake(game.enemyptrs.lock, 0) == pdTRUE) {
        tmpwall = game.enemyptrs.enemies[id[0]][id[1]];
        tumDrawFilledBox(tmpwall->x1 - 5, tmpwall->y1, tmpwall->w + 10, tmpwall->h + 5, Black);
        setWallProperty(tmpwall, INACTIVE, INACTIVE, NULL, NULL, SET_WALL_X); 
        xSemaphoreGive(game.enemyptrs.lock);
    }

    if(xSemaphoreTake(game.enemystates.lock, 0) == pdTRUE) {
        game.enemystates.states[id[0]][id[1]] = 0;
        xSemaphoreGive(game.enemystates.lock);
    }

    if(xSemaphoreTake(game.gamespeed.lock, 0) == pdTRUE) {
        if(game.gamespeed.gamespeed < BASEGAMETICK )
            game.gamespeed.gamespeed += speedincrease;
        xSemaphoreGive(game.gamespeed.lock);
    }

    renderHeader();
}

void ballOnCollision(int* id)
{
    ball_t* tmpball = NULL;
    
    if(xSemaphoreTake(game.projectileptrs.lock, 0) == pdTRUE) {
        tmpball = game.projectileptrs.list[id[0]];
        //queueRedCircle();
        xSemaphoreGive(game.projectileptrs.lock);
    }

    if(xSemaphoreTake(game.projectilestates.lock, 0) == pdTRUE) {
        game.projectilestates.states[id[0]] = 0;
        //queueRedCircle();
        xSemaphoreGive(game.projectilestates.lock);
    }

    tumDrawCircle(tmpball->x, tmpball->y, tmpball->radius, Black);
    setBallLocation(tmpball, INACTIVE, INACTIVE);
    setBallSpeed(tmpball, 0, 0, PROJECTILESPEED, SET_BALL_SPEED_ALL);
}

void addPlayerProjectile(int x, int y, int direction, unsigned int color, int radius, 
    int speed, int maxspeed)
{
    // int active = 0;
    // ball_t* shot = NULL;
    
    if(xSemaphoreTake(game.playershot.lock, 0) == pdTRUE) {
        if(game.playershot.active == 0) {
            setBallLocation(game.playershot.shot, x, y);
            setBallSpeed(game.playershot.shot, 0, direction * speed, 
                    maxspeed, SET_BALL_SPEED_Y);
            game.playershot.active = 1;
        }

        xSemaphoreGive(game.playershot.lock);
    }
}

void addProjectile(int x, int y, int direction, unsigned int color, int radius, 
    int speed, int maxspeed)
{   
    if(xSemaphoreTake(game.projectileptrs.lock, 0) == pdTRUE) {
        if(xSemaphoreTake(game.projectilestates.lock, 0) == pdTRUE) {
            for(int i = 0; i < MAXPROJECTILES; i++) {
                if(game.projectilestates.states[i] == 0) {
                    setBallLocation(game.projectileptrs.list[i], x, y);
                    setBallSpeed(game.projectileptrs.list[i], 0, direction * speed, 
                            maxspeed, SET_BALL_SPEED_Y);
                    game.projectilestates.states[i] = 1;
                    break;
                }
            }
            xSemaphoreGive(game.projectilestates.lock);        
        }
        xSemaphoreGive(game.projectileptrs.lock);
    }
}

void handleProjectiles()
{
    ball_t* tmpballs[MAXPROJECTILES] = { NULL };
    ball_t* playershot = NULL;

    if(xSemaphoreTake(game.playershot.lock, 0) ==pdTRUE) {
        if(game.playershot.active == 1) {
            playershot = game.playershot.shot;
            tumDrawCircle(playershot->x, playershot->y,
                    playershot->radius, Black);
            updateBallPosition(playershot, FRAMETIME); 
            tumDrawCircle(playershot->x, playershot->y, playershot->radius, PROJECTILECOLOR);
        }
        xSemaphoreGive(game.playershot.lock);
    }
    
    if(xSemaphoreTake(game.projectileptrs.lock, 0) == pdTRUE) {
        for(int i = 0; i < MAXPROJECTILES; i++) {
            tmpballs[i] = game.projectileptrs.list[i];
        }
        xSemaphoreGive(game.projectileptrs.lock);
    }
    
    for(int i = 0; i < MAXPROJECTILES; i++) {
        if(tmpballs[i]->x != INACTIVE) {
            tumDrawCircle(tmpballs[i]->x, tmpballs[i]->y,
                    tmpballs[i]->radius, Black);
            updateBallPosition(tmpballs[i], FRAMETIME);      
            tumDrawCircle(tmpballs[i]->x, tmpballs[i]->y,
                    tmpballs[i]->radius, tmpballs[i]->colour);
        }
    }
    
}

void renderFooter(int lives, int level, int inf_lives)
{
    char levelstr[16] = "LEVEL ";
    int w = 0, h = 0;
    
    tumDrawFilledBox(0, SCREEN_HEIGHT - FOOTERHEIGHT - GROUND_THICKNESS, SCREEN_WIDTH, GROUND_THICKNESS, Green); 
    
    tumDrawFilledBox(0, SCREEN_HEIGHT - FOOTERHEIGHT, SCREEN_WIDTH, FOOTERHEIGHT, Black);

    if (inf_lives) {
        tumDrawSprite(spritesheet, 8, 2, 20, SCREEN_HEIGHT - 75);
    }
    else {
        for(int i = 2; i > 0; i--)
            tumDrawSprite(spritesheet, 4, 2, 50 + (i - 2) * 50, SCREEN_HEIGHT - 80);
    }

    sprintf(&(levelstr[6]), "%.2d", level);
    tumGetTextSize("LEVEL 01", &w, &h);
    tumDrawText(levelstr, SCREEN_WIDTH - w - 10, SCREEN_HEIGHT - h - 20, White);


}

void clearScreen()
{
    tumDrawClear(Black);
}

void exitGraphics()
{
    tumDrawExit();
}