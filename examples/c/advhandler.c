#include "../../src/miceapi_main.h"
#include "../../src/miceapi_events.h"
#include <stdio.h>

//Copyright (c) 2020 Amélia O. F. da S.

/*
Demonstration of miceapi_advhandler usage
*/

int main()
{
    miceapi_device *mouse;
    miceapi_device *keyboard;
    miceapi_handler *mouseh;
    miceapi_advhandler *keyboardh;
    if(miceapi_create_device("/dev/input/event5",&mouse))return -1;//replace event5 with your mouse device
    if(miceapi_create_device("/dev/input/event3",&keyboard))//replace event3 with your keyboard device
    {
        miceapi_free_device(&mouse);
        return -1;
    }

    mouseh=miceapi_addhandler(mouse);
    keyboardh=miceapi_addadvhandler(keyboard);

    printf("Please press down the \"space\" key on your keyboard five times...\n");

    int p=0;
    int f=fork();
    miceapi_event evt;
    struct input_event ievt={0};
    if(f==0)//Mouse thread
    {
        int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (r == -1) {exit(1);}
        while(1)
        {
            evt=miceapi_wait_handler(mouseh);
            switch (evt&miceapi_MOVEMENT)
            {
                case miceapi_MOUSEMDOWN:
                    printf("V      \r");
                    fflush(stdout);
                    break;
                case miceapi_MOUSEMUP:
                    printf("^      \r");
                    fflush(stdout);
                    break;
                case miceapi_MOUSEMLEFT:
                    printf("<      \r");
                    fflush(stdout);
                    break;
                case miceapi_MOUSEMRIGHT:
                    printf(">      \r");
                    fflush(stdout);
                    break;
            }
        }
    }

    //Main thread
    while(p<5)
    {
        ievt=miceapi_wait_advhandler(keyboardh);
        if(ievt.type==1&&ievt.code==57&&ievt.value==1)//Spacebar down event
            p++;
    }

    miceapi_free_device(&mouse);
    miceapi_free_device(&keyboard);
    printf("\rOk!\n");
    return 0;
}