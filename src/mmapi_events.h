#ifndef MMAPI_EVENTS
#define MMAPI_EVENTS

#ifndef MMAPI_MAIN
#include "mmapi_main.h"
#endif

//Copyright (c) 2020 Amélia O. F. da S.

//Handler types
#define MMAPI_H_RAW 00//Raw handler
#define MMAPI_H_DEC 01//Decoded (mouse and trackpad) handler

//Represents a thread waiting for device events
typedef struct _mmapi_handler {
    int pipe[2];
    unsigned short int id;
    char type;
    int shm;//Shared memory key
    int next;//shmid
    char oc;//Occupied
} mmapi_handler;

/*
    Adds a handler to <mmapi_device *device>'s stack.
    Returns the handler object.
*/
mmapi_handler *mmapi_addhandler(mmapi_device *device);
/*
    Waits for an answer at handler <mmapi_handler *hand>.
    (Pre-decoded)
*/
mmapi_event mmapi_wait_handler(mmapi_handler *hand);
/*
    Internal function. Recursively frees handlers
*/
int mmapi_free_handlers(int handler);
#endif