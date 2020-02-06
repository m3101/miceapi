#include "../../src/miceapi_main.h"
#include "../../src/miceapi_events.h"
#include <stdio.h>

//Copyright (c) 2020 Amélia O. F. da S.

/*
A simple program to demonstrate suggested handler usage (on-demand creation)
This kind of implementation is the one baked into the python module wrapper
*/

void waitForClick(miceapi_device *device)
{
    miceapi_handler *clickhandler;
    miceapi_event evt=0;
    if(!device)return;
    clickhandler=miceapi_addhandler(device);
    if(!clickhandler)return;
    while(!(evt&miceapi_LCLICKUP))
        evt=miceapi_wait_handler(clickhandler);
    miceapi_remove_handler(device,clickhandler->id);
}

int main()
{
    miceapi_device *mouse;
    miceapi_create_device("/dev/input/event5",&mouse);//event5 happens to be my mouse. Substitute for yours (find device names with something like names.c).
    if(!mouse){printf("Could not open device. Check permissions.");return 0;}
    printf("This program demonstrates the suggested handler structure to be used.\nClick to continue...\n");
    waitForClick(mouse);
    printf("Following this architecture makes for a much more understandable program.\nClick to finish execution.\n");
    waitForClick(mouse);
    miceapi_free_device(&mouse);
    return 0;
}