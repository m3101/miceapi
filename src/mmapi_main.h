#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif
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
#include <errno.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <linux/input.h>

/*
Copyright (c) 2020 Amélia O. F. da S.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

//Pipe command flags
#define MMAPI_C_CLOSE 01

//Device error flags
#define MMAPI_E_NULLPOINTER 01 //Pointers that shouldn't be null are null (check arguments)
#define MMAPI_E_PIPE 02 //Pipe opening error
#define MMAPI_E_PATH 04 //Refers to errors when opening files (not strictly related to wrong paths)
#define MMAPI_E_SHM 010 //Refers to errors related to SHM
#define MMAPI_E_ACCESS 020 //Refers to errors related to access authorization (files and shm)

//Event codes
#define MMAPI_MOUSEMUP 01
#define MMAPI_MOUSEMDOWN 02
#define MMAPI_MOUSEMRIGHT 04
#define MMAPI_MOUSEMLEFT 010
#define MMAPI_LCLICKDOWN 020
#define MMAPI_LCLICKUP 040
#define MMAPI_RCLICKDOWN 0100
#define MMAPI_RCLICKUP 0200
#define MMAPI_MCLICKDOWN 0400
#define MMAPI_MCLICKUP 01000
#define MMAPI_SCROLLUP 02000
#define MMAPI_SCROLLDOWN 04000
#define MMAPI_UPDATEPOS 010000
#define MMAPI_OTHER 0200000 //For keyboard events, for example

//Event combination codes
#define MMAPI_MOVEMENT 017
#define MMAPI_CLICKDOWN 0520
#define MMAPI_CLICKUP 01240
#define MMAPI_SCROLL 03000

/*
    A macro for unpacking mmapi_event objects into a code(16bit) and a value(15bit)
*/
#define mmapi_unpackevt(event,uint16_code,int_value)\
                        uint16_code=(event<<((sizeof(event)*8)-16))>>((sizeof(event)*8)-16));\
                        int_value=event>>17;

//Thanks to maxschlepzig for this safety mechanism [https://stackoverflow.com/a/36945270]
#define diewithparent(pid)\
        int r = prctl(PR_SET_PDEATHSIG, SIGTERM);\
        if (r == -1) {  exit(1); }\
        if (getppid() != pid)\
          exit(1);\

/*
    A structure to abstract the pipe layout and file descriptors
    (Less useful now, but important nonetheless)
*/
typedef struct _mmapi_device{
    int fd; //File descriptor 
    char name[256];
    int x;//For trackpads only
    int y;//For trackpads only
    int shm;//For the event handler
    int hid;//For the event handler
    int ashm;//For the raw event handler
    int ahid;//For the raw event handler
    int selfshm;//For freeing shm later on
    int id;//For creating the shm key
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
int mmapi_free_device(mmapi_device **device);

/*
    Internal function. It works as a strncpy that copies exactly n bytes (regardless of null termination)
*/
void mmapi_bufncpy(void*dest,void*src,int n);

#endif