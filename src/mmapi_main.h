#define _GNU_SOURCE
#ifndef MMAPI_MAIN
#define MMAPI_MAIN
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <poll.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <linux/input.h>

//Copyright (c) 2020 Amélia O. F. da S.

//Pipe command flags
#define MMAPI_C_CLOSE 01

//Device error flags
#define MMAPI_E_NULLPOINTER 01 //Pointers that shouldn't be null are null (check arguments)
#define MMAPI_E_PIPE 02 //Pipe opening error
#define MMAPI_E_PATH 04 //Refers to errors when opening files (not strictly related to wrong paths)

//Event codes
#define MMAPI_MOUSEMUP 01
#define MMAPI_MOUSEMDOWN 02
#define MMAPI_MOUSEMRIGHT 04
#define MMAPI_MOUSEMLEFT 010
#define MMAPI_LCLICKDOWN 011
#define MMAPI_LCLICKUP 012
#define MMAPI_RCLICKDOWN 011
#define MMAPI_RCLICKUP 014
#define MMAPI_MCLICKDOWN 020
#define MMAPI_MCLICKUP 021
#define MMAPI_SCROLLUP 022
#define MMAPI_SCROLLDOWN 024
#define MMAPI_UPDATEPOS 030

//Macros (don't use)
#define _mmapi_close_ctl(device)\
    close(device->ctl[0]);close(device->ctl[1]);\
    device->ctl[0]=device->ctl[1]=NULL;
#define _mmapi_close_evt(device)\
    close(device->evt[0]);close(device->evt[1]);\
    device->evt[0]=device->evt[1]=NULL;
//Thanks to maxschlepzig for this safety mechanism [https://stackoverflow.com/a/36945270]
#define diewithparent(pid)\
        int r = prctl(PR_SET_PDEATHSIG, SIGTERM);\
        if (r == -1) { perror(0); exit(1); }\
        if (getppid() != pid)\
          exit(1);\

/*
    A structure to abstract the pipe layout and file descriptors
    (Less useful now, but important nonetheless)
*/
typedef struct _mmapi_device{
    int fd; //File descriptor 
    char name[256];
    int ctl[2]; //Pipe going from main to device
    int x;//For trackpads only
    int y;//For trackpads only
    int shm;//For the event handler
    int selfshm;//For freeing shm later on
} mmapi_device;
//A type to abstract input_events
typedef unsigned int mmapi_event;

/*
    Creates and starts monitoring a new device at <char* path>
*/
int mmapi_create_device(char* path,mmapi_device **device);
/*
    Lists the <int len> first available device names up to <int name_size> characters
    in <char** names> and their paths up to <int path_size> characters at <char** paths>
*/
int mmapi_available_names(char** names,char** paths,int len,int name_size,int path_size);
/*
    Internal function. Prepares the monitoring environment.
*/
int mmapi_start(mmapi_device *device,char *path);
/*
    Internal function. Starts the monitoring thread.
*/
int mmapi_start_thread(mmapi_device *device);
/*
    Decodes an <input_event *evt> to the simpler mmapi_event format.
    Needs a <mmapi_device *device> for calculating movement with trackpads.
*/
mmapi_event mmapi_decode(mmapi_device *device,struct input_event *evt);
/*
    Frees a device.
*/
int mmapi_free_device(mmapi_device *device);
#endif