#include "sound.h"
#include "TUM_Sound.h"
#include <stdio.h>
// #include "TUM_Print.h"
// #include "TUM_Utils.h"

int initSound(char* bin_folder_path)
{      
    if (tumSoundInit(bin_folder_path)) {
        printf("Failed to initialize audio");
        return -1;
    }
    else {
        printf("Audio Initialized.\n");
        return 0;
    }
}

void exitSound()
{
    tumSoundExit();
}