#ifndef MMAPI_EVENTS
#define MMAPI_EVENTS

#ifndef MMAPI_MAIN
#include "mmapi_main.h"
#endif

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

//Shared memory id initial number
#define MMAPI_H_SHMID 20

//Handler types
#define MMAPI_H_DEC 01//Decoded (mouse and trackpad) handler
#define MMAPI_H_RAW 02//Raw handler

//Constants
#define MMAPI_H_BUFSIZ 8

//Represents a thread waiting for device events
typedef struct _mmapi_handler {
    char buffer[sizeof(mmapi_event)*MMAPI_H_BUFSIZ];//Command buffer
    int cc;//Current command position on buffer
    int ec;//Last command position on buffer
    unsigned short int id;
    char type;
    int shm;//Shared memory id
    int next;//Next handler shmid
    int nid;//Next handler id for shm key generation
    char oc;//Occupied
} mmapi_handler;

//Represents a thread waiting for raw device events
typedef struct _mmapi_advhandler {
    char buffer[sizeof(struct input_event)*MMAPI_H_BUFSIZ];//Command buffer
    int cc;//Current command position on buffer
    int ec;//Last command position on buffer
    unsigned short int id;
    char type;
    int shm;//Shared memory id
    int next;//Next handler shmid
    int nid;//Next handler id for shm key generation
    char oc;//Occupied
} mmapi_advhandler;

/*
    Adds a handler to <mmapi_device *device>'s stack.
    Returns the handler object.
*/
mmapi_handler *mmapi_addhandler(mmapi_device *device);
/*
    Adds an advanced (raw) handler to <mmapi_device *device>'s stack.
    Returns the adv_handler object.
*/
mmapi_advhandler *mmapi_addadvhandler(mmapi_device *device);
/*
    Detaches the handler with id <int id> from device <mmapi_device *device>
*/
int mmapi_remove_handler(mmapi_device *device,int id);
/*
    Detaches the raw handler with id <int id> from device <mmapi_device *device>
*/
int mmapi_remove_advhandler(mmapi_device *device,int id);
/*
    Waits for an answer at handler <mmapi_handler *hand>.
    (Pre-decoded)
*/
mmapi_event mmapi_wait_handler(mmapi_handler *hand);
/*
    Waits for an answer at handler <mmapi_advhandler *hand>.
    (Raw)
*/
struct input_event mmapi_wait_advhandler(mmapi_advhandler *hand);
/*
    Internal function. Recursively frees handlers
*/
int mmapi_free_handlers(int handler,int hid);
/*
    Internal function. Recursively frees raw handlers
*/
int mmapi_free_advhandlers(int handler,int hid);
#endif