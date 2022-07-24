#include <SDL2/SDL_scancode.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define NUMOFKEYS 7
#define ENTER   40
#define ESC     41
#define SPACE   44
#define RIGHT   79
#define LEFT    80
#define DOWN    81
#define UP      82
#define ENTER_BIT   0x0001
#define ESC_BIT     0x0002
#define SPACE_BIT   0x0004
#define RIGHT_BIT   0x0008
#define LEFT_BIT    0x0010
#define DOWN_BIT    0x0020
#define UP_BIT      0x0040

void exitInput();