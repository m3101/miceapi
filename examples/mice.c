#include "../src/mmapi_main.h"
#include "../src/mmapi_events.h"
#include <stdio.h>

//Copyright (c) 2020 Amélia O. F. da S.

int main()
{
    mmapi_device *mouse,*pad;
    unsigned int stat=mmapi_create_device("/dev/input/event5",&mouse);
    mmapi_handler *mouseevents;
    mmapi_event evt;
    if(stat!=0)
    {
        printf("mice.c: could not create device (error %u). Check permissions.\n",stat);
        return 0;
    }
    //if(!mmapi_create_device("/dev/input/event4",pad));
    printf("Devices created.\n");
    printf("Mouse name is %s\n",mouse->name);
    mouseevents=mmapi_addhandler(mouse);
    evt=0;
    printf("Waiting for click...");
    while(!(evt&MMAPI_LCLICKDOWN))
    {
        evt=mmapi_wait_handler(mouseevents);
    }
    printf("Done!\n");
    return 0;
}