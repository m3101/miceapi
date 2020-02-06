#include "../src/miceapi_main.h"
#include <stdio.h>

//Copyright (c) 2020 Amélia O. F. da S.

/*
Demonstrates a possible use for miceapi_device_names
*/

int main()
{
    char *paths[32];
    char *names[32];
    int devices,i,fd;
    fd=open("/dev/input/event0",O_NONBLOCK|O_RDONLY);
    if(fd<=0)
    {
        printf("names.c: cannot open devices. Check permissions (maybe run as superuser).\n");
        return 0;
    }
    for(i=0;i<32;i++)
    {
        paths[i]=malloc(256*sizeof(char));
        names[i]=malloc(256*sizeof(char));
    }
    devices=miceapi_available_names(names,paths,32,256,256);
    for(i=0;i<devices;i++)printf("Device %s at %s\n",names[i],paths[i]);
    for(i=0;i<32;i++)
    {
        free(paths[i]);
        free(names[i]);
    }
    return 0;
}