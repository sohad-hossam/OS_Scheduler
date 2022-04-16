#include "headers.h"
#include <stdlib.h>
#include <time.h>

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char *argv[])
{
    //initClk();
    //int algorithm = atoi(argv[2]);
    remainingtime = atoi(argv[1]);
    printf("The running time is %d\n",remainingtime);
    while ((clock()+100000) < atoi(argv[1]) * CLOCKS_PER_SEC);
   return 0;
    //TODO it needs to get the remaining time from somewhere
    //remainingtime = ??;
    //destroyClk(false);

  
}
