#include "mmapi_main.h"
#include "mmapi_events.h"

int mmapi_hid=1;
mmapi_handler *mmapi_addhandler(mmapi_device *device)
{
    if(!device)return NULL;
    key_t key=ftok(".",10+mmapi_hid);
    int shm=shmget(key,sizeof(mmapi_handler),IPC_CREAT);
    if(shm==-1)
    {
        return NULL;
    }
    int nexts;
    int i;
    mmapi_handler *hand=shmat(shm,(void*)0,0);
    mmapi_handler *next;
    if(!hand||hand==~0)return NULL;

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
    }
    strncpy(&hand->buffer[pos],&zero,sizeof(mmapi_event));//Zero the buffer location
    hand->cc++;
    hand->cc%=MMAPI_H_BUFSIZ;
    hand->oc=0;
    return evt;
}

int mmapi_free_handlers(int handler,int hid)
{
    key_t key=ftok(".",10+hid);
    int nexts;
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
}