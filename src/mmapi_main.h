#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <dirent.h>
#include <poll.h>

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

//Macros (don't use)
#define _mmapi_close_ctl(device)\
    close(device->ctl[0]);close(device->ctl[1]);\
    device->ctl[0]=device->ctl[1]=NULL;
#define _mmapi_close_evt(device)\
    close(device->evt[0]);close(device->evt[1]);\
    device->evt[0]=device->evt[1]=NULL;

//A structure to abstract the pipe layout and file descriptors
typedef struct _mmapi_device{
    int fd; //File descriptor 
    char name[256];
    int ctl[2]; //Pipe going from main to device
    int evt[2]; //Pipe going from device to main
} mmapi_device;
//A type to abstract input_events
typedef unsigned int mmapi_event;

//Lists the available device names and their paths 
int mmapi_available_names(char** names,char** paths,int len,int name_size,int path_size);
//Start monitoring the device at <path>. (device pointer should be unused)
int mmapi_start(mmapi_device *device,char* path);
int mmapi_start_thread(mmapi_device *device);
//Wait for an event coming from the device
mmapi_event mmapi_wait(mmapi_device *device);
//Advanced form of mmapi_wait. Return the raw input_event (see linux/input.h for reference)
struct input_event *mmapi_wait_adv(mmapi_device *device);
//Creates and starts monitoring a device at path <path> (device should be a null pointer)
int mmapi_create_device(char* path,mmapi_device** device);