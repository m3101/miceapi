#include "mmapi_main.h"
#include "mmapi_events.h"

#include <stdio.h>

//Copyright (c) 2020 Amélia O. F. da S.

int mmapi_hid=1;
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

int mmapi_remove_handler(mmapi_device *device,int id)
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

int mmapi_free_handlers(int handler,int hid)
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