#include "input.h"
#include "TUM_Event.h"
// #include "TUM_Utils.h"
// #include "TUM_Print.h"
#include <stdio.h>

int initInput()
{
    
    int init_fault = tumEventInit();

    if (init_fault) {
        printf("Failed to initialize events");
        return -1;
    }
    else {
        printf("Input handling intitialized.\n");
        return 0;
    }
}

void exitInput()
{
    tumEventExit();
}