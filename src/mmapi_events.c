#include "mmapi_main.h"
#include "mmapi_events.h"

#include <stdio.h>

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

unsigned int mmapi_hid=1;
mmapi_handler *mmapi_addhandler(mmapi_device *device)
{
    if(!device)return NULL;
    key_t key=ftok(".",MMAPI_H_SHMID+mmapi_hid);
    int shm=shmget(key,sizeof(mmapi_handler),IPC_CREAT);
    if(shm==-1)
    {
        return NULL;
    }
    int nexts;
    unsigned long int i;
    mmapi_handler *hand=shmat(shm,(void*)0,0);
    mmapi_handler *next;
    if(!hand||hand==(void*)~0)return NULL;
    hand->id=mmapi_hid++;
    mmapi_hid%=UINT32_MAX;
    hand->oc=0;
    hand->next=0;
    hand->type=MMAPI_H_DEC;
    hand->shm=shm;
    hand->cc=0;
    hand->ec=0;
    for(i=0;i<MMAPI_H_BUFSIZ*sizeof(mmapi_event);i++)
    {
        hand->buffer[i]=0;
    }
    if(device->shm)
    {
        next=shmat(device->shm,(void*)0,0);
        nexts=next->next;
        while(nexts)
        {
            shmdt(next);
            next=shmat(nexts,(void*)0,0);
            nexts=next->next;
        }
        next->next=shm;
        next->nid=hand->id;
        shmdt(next);
    }
    else
    {
        device->shm=shm;
        device->hid=hand->id;
    }
    return hand;
}

mmapi_advhandler *mmapi_addadvhandler(mmapi_device *device)
{
    if(!device)return NULL;
    key_t key=ftok(".",MMAPI_H_SHMID+mmapi_hid);
    int shm=shmget(key,sizeof(mmapi_advhandler),IPC_CREAT);
    if(shm==-1)
    {
        return NULL;
    }
    int nexts;
    unsigned long int i;
    mmapi_advhandler *hand=shmat(shm,(void*)0,0);
    mmapi_advhandler *next;
    if(!hand||hand==(void*)~0)return NULL;
    hand->id=mmapi_hid++;
    mmapi_hid%=UINT32_MAX;
    hand->oc=0;
    hand->next=0;
    hand->type=MMAPI_H_RAW;
    hand->shm=shm;
    hand->cc=0;
    hand->ec=0;
    for(i=0;i<MMAPI_H_BUFSIZ*sizeof(struct input_event);i++)
    {
        hand->buffer[i]=0;
    }
    if(device->ashm)
    {
        next=shmat(device->ashm,(void*)0,0);
        nexts=next->next;
        while(nexts)
        {
            shmdt(next);
            next=shmat(nexts,(void*)0,0);
            nexts=next->next;
        }
        next->next=shm;
        next->nid=hand->id;
        shmdt(next);
    }
    else
    {
        device->ashm=shm;
        device->ahid=hand->id;
    }
    return hand;
}

int mmapi_remove_handler(mmapi_device *device,unsigned int id)
{
    int shmid;
    if(!device)return -1;
    if(!device->shm)return -2;
    mmapi_handler *cur=shmat(device->shm,(void*)0,0),*prev=0;
    if(!cur)return -3;
    if(cur->id==id)
    {
        device->shm=cur->next;
        shmid=cur->shm;
        shmdt(cur);
        shmctl(shmid,IPC_RMID,0);
        return 0;
    }
    while(cur->id!=id)
    {
        if(prev)shmdt(prev);
        prev=cur;
        if(!cur->next)
        {
            shmdt(prev);
            shmdt(cur);
            return -4;
        }
        cur=shmat(cur->next,(void*)0,0);
    }
    prev->next=cur->next;
    shmdt(cur);
    shmid=cur->shm;
    shmctl(shmid,IPC_RMID,0);
    return 0;
}

int mmapi_remove_advhandler(mmapi_device *device,unsigned int id)
{
    int shmid;
    if(!device)return -1;
    if(!device->shm)return -2;
    mmapi_advhandler *cur=shmat(device->shm,(void*)0,0),*prev=0;
    if(!cur)return -3;
    if(cur->id==id)
    {
        device->shm=cur->next;
        shmid=cur->shm;
        shmdt(cur);
        shmctl(shmid,IPC_RMID,0);
        return 0;
    }
    while(cur->id!=id)
    {
        if(prev)shmdt(prev);
        prev=cur;
        if(!cur->next)
        {
            shmdt(prev);
            shmdt(cur);
            return -4;
        }
        cur=shmat(cur->next,(void*)0,0);
    }
    prev->next=cur->next;
    shmdt(cur);
    shmid=cur->shm;
    shmctl(shmid,IPC_RMID,0);
    return 0;
}

mmapi_event mmapi_wait_handler(mmapi_handler *hand)
{
    mmapi_event evt=0;
    mmapi_event zero=0;
    int pos;
    if(!hand||hand->oc)return 0;
    hand->oc=1;
    pos=hand->cc*sizeof(mmapi_event);
    while(evt==0)
    {
        evt=*((mmapi_event*)&hand->buffer[pos]);
        /*
        The python module was having problems when this checking was too fast.
        I don't really know why, but sleeping makes it work.
        I guess it has something to do with the shared memory.
        */
        usleep(100);
    }
    strncpy(&hand->buffer[pos],(char*)&zero,sizeof(mmapi_event));//Zero the buffer location
    hand->cc++;
    hand->cc%=MMAPI_H_BUFSIZ;
    hand->oc=0;
    return evt;
}

struct input_event empty_event={0};
struct input_event mmapi_wait_advhandler(mmapi_advhandler *hand)
{
    struct input_event evt=empty_event;
    int pos;
    if(!hand||hand->oc)return empty_event;
    hand->oc=1;
    pos=hand->cc*sizeof(struct input_event);
    while(evt.type==0)
    {
        mmapi_bufncpy(&evt,(char*)&hand->buffer[pos],sizeof(struct input_event));
        /*
        The python module was having problems when this checking was too fast.
        I don't really know why, but sleeping makes it work.
        I guess it has something to do with the shared memory.
        */
        usleep(100);
    }
    strncpy(&hand->buffer[pos],(char*)&empty_event,sizeof(struct input_event));//Zero the buffer location
    hand->cc++;
    hand->cc%=MMAPI_H_BUFSIZ;
    hand->oc=0;
    return evt;
}

int mmapi_free_handlers(int handler,unsigned int hid)
{
    mmapi_handler *hand;
    if(handler)
    {
        hand=shmat(handler,(void*)0,0);
        if(hand)
        {
            if(hand->next)
            {
                mmapi_free_handlers(hand->next,hand->nid);
            }
            shmdt(hand);
            shmctl(handler,IPC_RMID,0);
        }
    }
    return 0;
}

int mmapi_free_advhandlers(int handler,unsigned int hid)
{
    mmapi_advhandler *hand;
    if(handler)
    {
        hand=shmat(handler,(void*)0,0);
        if(hand)
        {
            if(hand->next)
            {
                mmapi_free_advhandlers(hand->next,hand->nid);
            }
            shmdt(hand);
            shmctl(handler,IPC_RMID,0);
        }
    }
    return 0;
}