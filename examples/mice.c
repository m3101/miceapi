#include "../src/mmapi_main.h"
#include <stdio.h>

//Copyright (c) 2020 Sérgio F. da S. J.

void _mouse(mmapi_device *mouse)
{
    mmapi_event evt;
    if(fork()>0)
    {
        return;
    }
    else
    {
        time_t start=time(NULL);
        while(time(NULL)-start<5)
        {
            evt = mmapi_wait(mouse);
        }
        printf("Closing.\n");
        write(mouse->ctl[1],MMAPI_C_CLOSE,sizeof(unsigned int));
    }
}

void _pad(mmapi_device *pad)
{
    return;
}

int main()
{
    mmapi_device *mouse,*pad;
    unsigned int stat=mmapi_create_device("/dev/input/event5",&mouse);
    if(stat!=0)
    {
        printf("mice.c: could not create device (error %u). Check permissions.\n",stat);
        return 0;
    }
    //if(!mmapi_create_device("/dev/input/event4",pad));
    printf("Devices created.\n");
    printf("Mouse name is %s\n",mouse->name);
    printf("Starting event listener...\n");
    _mouse(mouse);
    return 0;
}