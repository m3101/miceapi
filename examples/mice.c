#include "../src/mmapi_main.h"
#include "../src/mmapi_events.h"
#include <stdio.h>

//Copyright (c) 2020 Amélia O. F. da S.

int main()
{
    mmapi_device *mouse;
    unsigned int stat=mmapi_create_device("/dev/input/event5",&mouse);//event5 happened to be my mouse. Use names.c to find the available devices.
    mmapi_handler *clickwaiter;
    mmapi_handler *movetracker;
    mmapi_event evt;
    int waitid;
    if(stat!=0)
    {
        printf("mice.c: could not create device (errno %d). ", errno);
        if(stat&MMAPI_E_ACCESS)
        {
            if(stat&MMAPI_E_SHM) printf("Not authorized to use Shared Memory. Check permissions.\n");
            else printf("Not authorized to access device files. Check permissions.\n");
        }
        else if(stat&MMAPI_E_SHM)
        {
            printf("Shared memory error. Zombie memory might be leftover from crash. Clear memory blocks with size %ld (devices) and %ld (handlers) with ipcs and ipcrm.\n",sizeof(mmapi_device),sizeof(mmapi_handler));
        }
        return 0;
    }
    printf("Devices created.\n");
    printf("Mouse name is %s\n",mouse->name);
    clickwaiter=mmapi_addhandler(mouse);
    movetracker=mmapi_addhandler(mouse);
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
            evt=mmapi_wait_handler(movetracker);
            switch (evt)
            {
                case MMAPI_MOUSEMDOWN:
                    printf("V\r");
                    fflush(stdout);
                    break;
                case MMAPI_MOUSEMUP:
                    printf("^\r");
                    fflush(stdout);
                    break;
                case MMAPI_MOUSEMLEFT:
                    printf("<\r");
                    fflush(stdout);
                    break;
                case MMAPI_MOUSEMRIGHT:
                    printf(">\r");
                    fflush(stdout);
                    break;
            }
        }
    }

    //This thread, the main thread, will wait for a mouse click to die
    evt=0;
    printf("Main thread waiting for click...\n");
    while(!(evt&MMAPI_LCLICKDOWN))
    {
        evt=mmapi_wait_handler(clickwaiter);
    }
    printf("Closing everything...\n");
    mmapi_free_device(&mouse);
    return 0;
}