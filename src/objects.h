#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include <SDL2/SDL_scancode.h>
#include "TUM_Ball.h"

#ifndef OBJECTS_H_
#define OBJECTS_H_

#define NUMOFHIGHSCORES 5
#define MAXPROJECTILES 10

#define ENEMYCOL 11
#define ENEMYROW 5

typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
} buttons_buffer_t;

// typedef struct keystate {
//     unsigned char keystate[NUMOFKEYS];
//     SemaphoreHandle_t lock;
// } keystate;
// static keystate last_keystate;

// State type; Mutex to store state ids. Used for High level state machine
// and various states in different screens
typedef struct statechar{
    int state;
    SemaphoreHandle_t lock;
} statechar;

struct isgameactive{
    char isit;
    SemaphoreHandle_t lock;
};

typedef struct gameparammutex{
    char level;
    long int score;
    char lives;
    char inf_lives;
    SemaphoreHandle_t lock;
} gameparammutex;

struct gameparameters{
    char level;
    long int score;
    char lives;
    char inf_lives;
};

typedef struct hs{
    long int highscores[5];
    SemaphoreHandle_t lock;
} hs;


static struct game{
    struct gamespeed{
        float_t gamespeed;
        SemaphoreHandle_t lock;
    }gamespeed;
    struct position{
        int position;
        SemaphoreHandle_t lock;
    }position;
    struct enemyptrs{
        wall_t* enemies[ENEMYROW][ENEMYCOL];
        SemaphoreHandle_t lock;
    }enemyptrs;
    struct playershot{
        char active;
        ball_t* shot;
        SemaphoreHandle_t lock;
    }playershot;
    struct projectileptrs{
        ball_t* list[MAXPROJECTILES];
        SemaphoreHandle_t lock;
    }projectileptrs;
    struct enemystates{
        char states[ENEMYROW][ENEMYCOL];
        SemaphoreHandle_t lock;
    }enemystates;
    struct projectilestates{
        char states[MAXPROJECTILES];
        SemaphoreHandle_t lock;
    }projectilestates;
};


static struct playerhitbox{
    wall_t* box;
    SemaphoreHandle_t lock;
};



#endif //OBJECTS_H_