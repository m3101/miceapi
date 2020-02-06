#include "../../src/miceapi_main.h"
#include "../../src/miceapi_events.h"
#include <stdio.h>

//Copyright (c) 2020 Amélia O. F. da S.

/*
A simple program to showcase the API's main functionalities.
It tracks the device at event5 (change it to one of your pointer devices)
*/

int main()
{
    miceapi_device *mouse;
    unsigned int stat=miceapi_create_device("/dev/input/event5",&mouse);//event5 happened to be my mouse. Use names.c to find the available devices.
    miceapi_handler *clickwaiter;
    miceapi_handler *movetracker;
    miceapi_event evt;
    int waitid;
    if(stat!=0)
    {
        printf("mice.c: could not create device (errno %d). ", errno);
        if(stat&miceapi_E_ACCESS)
        {
            if(stat&miceapi_E_SHM) printf("Not authorized to use Shared Memory. Check permissions.\n");
            else printf("Not authorized to access device files. Check permissions.\n");
        }
        else if(stat&miceapi_E_SHM)
        {
            printf("Shared memory error. Zombie memory might be leftover from crash. Clear memory blocks with size %ld (devices) and %ld (handlers) with ipcs and ipcrm.\n",sizeof(miceapi_device),sizeof(miceapi_handler));
        }
        return 0;
    }
    printf("Devices created.\n");
    printf("Mouse name is %s\n",mouse->name);
    clickwaiter=miceapi_addhandler(mouse);
    movetracker=miceapi_addhandler(mouse);
    if(!clickwaiter||!movetracker)
    {
        printf("Could not create handler. Error: %d.\n",errno);
        return 0;
    }

    //Now we'll separate two threads to demonstrate two possible uses of the event handler

    waitid=fork();
    if(!waitid)
    {
        //This thread will keep track of the mouse movements.
        //These lines ensure the thread will die too when its parent dies
        int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (r == -1) { perror(0); exit(1); }
        printf("Side thread started.\n");
        while(1)
        {
            evt=miceapi_wait_handler(movetracker);
            switch (evt&miceapi_MOVEMENT)
            {
                case miceapi_MOUSEMDOWN:
                    printf("V\r");
                    fflush(stdout);
                    break;
                case miceapi_MOUSEMUP:
                    printf("^\r");
                    fflush(stdout);
                    break;
                case miceapi_MOUSEMLEFT:
                    printf("<\r");
                    fflush(stdout);
                    break;
                case miceapi_MOUSEMRIGHT:
                    printf(">\r");
                    fflush(stdout);
                    break;
            }
        }
    }

    //This thread, the main thread, will wait for a mouse click to die
    evt=0;
    printf("Main thread waiting for click...\n");
    while(!(evt&miceapi_LCLICKUP))
    {
        evt=miceapi_wait_handler(clickwaiter);
    }
    printf("Closing everything...\n");
    miceapi_free_device(&mouse);
    return 0;
}