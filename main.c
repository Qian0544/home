#include <stdio.h>
#include <time.h>
#include "UI.h"


int main() {
    srand(time(NULL)); 
    /*ensure different keys (rand) every time run the program*/

    return diaryMenuLoop();
}


    
