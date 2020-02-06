#ifndef miceapi_EVENTS
#define miceapi_EVENTS

#ifndef miceapi_MAIN
#include "miceapi_main.h"
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
#define miceapi_H_SHMID 20

//Handler types
#define miceapi_H_DEC 01//Decoded (mouse and trackpad) handler
#define miceapi_H_RAW 02//Raw handler

//Constants
#define miceapi_H_BUFSIZ 8

//Represents a thread waiting for device events
typedef struct _miceapi_handler {
    char buffer[sizeof(miceapi_event)*miceapi_H_BUFSIZ];//Command buffer
    int cc;//Current command position on buffer
    int ec;//Last command position on buffer
    unsigned int id;
    char type;
    int shm;//Shared memory id
    int next;//Next handler shmid
    int nid;//Next handler id for shm key generation
    char oc;//Occupied
} miceapi_handler;

//Represents a thread waiting for raw device events
typedef struct _miceapi_advhandler {
    char buffer[sizeof(struct input_event)*miceapi_H_BUFSIZ];//Command buffer
    int cc;//Current command position on buffer
    int ec;//Last command position on buffer
    unsigned int id;
    char type;
    int shm;//Shared memory id
    int next;//Next handler shmid
    int nid;//Next handler id for shm key generation
    char oc;//Occupied
} miceapi_advhandler;

/*
    Adds a handler to <miceapi_device *device>'s stack.
    Returns the handler object.
*/
miceapi_handler *miceapi_addhandler(miceapi_device *device);
/*
    Adds an advanced (raw) handler to <miceapi_device *device>'s stack.
    Returns the adv_handler object.
*/
miceapi_advhandler *miceapi_addadvhandler(miceapi_device *device);
/*
    Detaches the handler with id <int id> from device <miceapi_device *device>
*/
int miceapi_remove_handler(miceapi_device *device,unsigned int id);
/*
    Detaches the raw handler with id <int id> from device <miceapi_device *device>
*/
int miceapi_remove_advhandler(miceapi_device *device,unsigned int id);
/*
    Waits for an answer at handler <miceapi_handler *hand>.
    (Pre-decoded)
*/
miceapi_event miceapi_wait_handler(miceapi_handler *hand);
/*
    Waits for an answer at handler <miceapi_advhandler *hand>.
    (Raw)
*/
struct input_event miceapi_wait_advhandler(miceapi_advhandler *hand);
/*
    Internal function. Recursively frees handlers
*/
int miceapi_free_handlers(int handler,unsigned int hid);
/*
    Internal function. Recursively frees raw handlers
*/
int miceapi_free_advhandlers(int handler,unsigned int hid);
#endif