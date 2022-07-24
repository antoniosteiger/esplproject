#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

#include "TUM_Ball.h"
#include "TUM_Utils.h"
#include "TUM_FreeRTOS_Utils.h"
#include "TUM_Print.h"
//#include "TUM_Draw.h"

#include "objects.h"
#include "sound.h"
#include "graphics.h"
#include "logic.h"
#include "input.h"


//#include "AsyncIO.h"

//Defines
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)
#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)

gameparammutex parameters;
statechar state;
hs highscores;
buttons_buffer_t buttons;
struct game game;
struct playerhitbox playerhitbox;
struct isgameactive isgameactive;

int enemyid[ENEMYROW][ENEMYCOL][2] = { 0 };
int projectileid[MAXPROJECTILES][2] = { 0 };

// Semaphores
// typedef struct buttons_buffer {
//     unsigned char buttons[SDL_NUM_SCANCODES];
//     SemaphoreHandle_t lock;
// } buttons_buffer_t;

// static buttons_buffer_t buttons = { 0 };

//Timers
// TimerHandle_t timer1handle = NULL;


//Task Handles
//static TaskHandle_t DrawTask = NULL;


// void vprintTask(xTimerHandle timer1)
// {
//     tumDrawCircle(100,100,50,TUMBlue);
// }

// void vDrawTask()
// {
//     tumDrawBindThread();
//     while(1) {
//         // tumDrawClear(White);
//         tumDrawUpdateScreen();
//         vTaskDelay(pdMS_TO_TICKS(20));
//     }
// }

int main(int argc, char *argv[])
{
    char *bin_folder_path = tumUtilGetBinFolderPath(argv[0]);
    // if (safePrintInit()) {
    //     PRINT_ERROR("Failed to init safe print");
    //     goto err_init_safe_print;
    // }

    parameters.lock = xSemaphoreCreateMutex();
    state.lock = xSemaphoreCreateMutex();
    highscores.lock = xSemaphoreCreateMutex();
    buttons.lock = xSemaphoreCreateMutex();

    isgameactive.lock = xSemaphoreCreateMutex();
    if(xSemaphoreTake(isgameactive.lock, 0) == pdTRUE) {
        isgameactive.isit = 0;
        xSemaphoreGive(isgameactive.lock);
    }

    game.position.lock = xSemaphoreCreateMutex();
    game.playershot.lock = xSemaphoreCreateMutex();
    game.gamespeed.lock = xSemaphoreCreateMutex();
    playerhitbox.lock = xSemaphoreCreateMutex();
    game.projectileptrs.lock = xSemaphoreCreateMutex(); 
    game.enemyptrs.lock = xSemaphoreCreateMutex();
    game.projectilestates.lock = xSemaphoreCreateMutex();
    game.enemystates.lock = xSemaphoreCreateMutex();
    game.position.lock = xSemaphoreCreateMutex();
    game.playershot.lock = xSemaphoreCreateMutex();
    
    //Initializations:
    prints("Initializing: \n");

    if (initLogic() == -1)
        goto err_init_logic;
    
    if (initGraphics(bin_folder_path) == -1)
        goto err_init_graphics;

    if (initInput() == -1)
        goto err_init_input;

    if (initSound(bin_folder_path) == -1)
        goto err_init_sound;



    // // Semaphores:
    // buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism
    // if (!buttons.lock) {
    //     PRINT_ERROR("Failed to create buttons lock");
    //     goto err_buttons_lock;
    // }

    //Timers
    // timer1handle = xTimerCreate("timer1",
    //                pdMS_TO_TICKS(1000),
    //                pdTRUE,
    //                (void*) 0,
    //                vprintTask);
    // if (timer1handle==NULL) {
    //     for(;;); /* failure! */
    // }
    // if (xTimerStart(timer1handle, 0)!=pdPASS) {
    //     for(;;); /* failure!?! */
    // }

    handleStartScreen(0x000);
    vTaskStartScheduler();

    return EXIT_SUCCESS;

err_init_logic:
    exitSound();
err_init_sound:
    exitInput();
err_init_input:
    exitGraphics();
err_init_graphics:
    safePrintExit();
err_init_safe_print:
    return EXIT_FAILURE;
}































//***************************************************************************//
// cppcheck-suppress unusedFunction
__attribute__((unused)) void vMainQueueSendPassed(void)
{
    /* This is just an example implementation of the "queue send" trace hook. */
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
    struct timespec xTimeToSleep, xTimeSlept;
    /* Makes the process more agreeable when using the Posix simulator. */
    xTimeToSleep.tv_sec = 1;
    xTimeToSleep.tv_nsec = 0;
    nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}
