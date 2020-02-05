#include "../src/mmapi_main.h"
#include "../src/mmapi_events.h"
#include <stdio.h>

//Copyright (c) 2020 Amélia O. F. da S.

/*
Demonstration of mmapi_advhandler usage
*/

int main()
{
    mmapi_device *mouse;
    mmapi_device *keyboard;
    mmapi_handler *mouseh;
    mmapi_advhandler *keyboardh;
    if(mmapi_create_device("/dev/input/event5",&mouse))return -1;//replace event5 with your mouse device
    if(mmapi_create_device("/dev/input/event3",&keyboard))//replace event3 with your keyboard device
    {
        mmapi_free_device(&mouse);
        return -1;
    }

    mouseh=mmapi_addhandler(mouse);
    keyboardh=mmapi_addadvhandler(keyboard);

    printf("Please press down the \"space\" key on your keyboard five times...\n");

    int p=0;
    int f=fork();
    mmapi_event evt;
    struct input_event ievt={0};
    if(f==0)//Mouse thread
    {
        int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (r == -1) {exit(1);}
        while(1)
        {
            evt=mmapi_wait_handler(mouseh);
            switch (evt&MMAPI_MOVEMENT)
            {
                case MMAPI_MOUSEMDOWN:
                    printf("V      \r");
                    fflush(stdout);
                    break;
                case MMAPI_MOUSEMUP:
                    printf("^      \r");
                    fflush(stdout);
                    break;
                case MMAPI_MOUSEMLEFT:
                    printf("<      \r");
                    fflush(stdout);
                    break;
                case MMAPI_MOUSEMRIGHT:
                    printf(">      \r");
                    fflush(stdout);
                    break;
            }
        }
    }

    //Main thread
    while(p<5)
    {
        ievt=mmapi_wait_advhandler(keyboardh);
        if(ievt.type==1&&ievt.code==57&&ievt.value==1)//Spacebar down event
            p++;
    }

    mmapi_free_device(&mouse);
    mmapi_free_device(&keyboard);
    printf("\rOk!\n");
    return 0;
}