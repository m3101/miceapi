#include "mmapi_main.h"
#include "mmapi_events.h"

int mmapi_hid=0;
mmapi_handler *mmapi_addhandler(mmapi_device *device)
{
    if(!device)return NULL;
    key_t key=ftok("shm",10);
    int shm=shmget(key,sizeof(mmapi_handler),IPC_CREAT);
    int nexts;
    mmapi_handler *hand=shmat(shm,(void*)0,0);
    mmapi_handler *next;
    if(!hand)return hand;
    hand->id=mmapi_hid++;
    hand->next=0;
    if(pipe(hand->pipe)==-1)
        exit(-1);
    printf("Handler pipe write address:%X\n",hand->pipe[1]);
    close(hand->pipe[1]);
    hand->type=MMAPI_H_DEC;
    hand->shm=shm;
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
        shmdt(next);
    }
    else device->shm=shm;
    printf("Handler created.");
    return hand;
}

mmapi_event mmapi_wait_handler(mmapi_handler *hand)
{
    mmapi_event evt;
    if(!hand||hand->oc)return 0;
    hand->oc=1;
    read(hand->pipe[0],&evt,sizeof(evt));
    hand->oc=0;
    return evt;
}

int mmapi_free_handlers(int handler)
{
    key_t key=ftok("shm",10);
    int nexts;
    mmapi_handler *hand;
    if(handler)
    {
        hand=shmat(handler,(void*)0,0);
        if(hand)
        {
            if(hand->next)
            {
                mmapi_free_handlers(hand->next);
            }
            shmdt(hand);
            shmctl(handler,IPC_RMID,0);
        }
    }
}