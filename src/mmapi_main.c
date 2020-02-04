#include "mmapi_main.h"
#include "mmapi_events.h"
#include <stdio.h>

//Copyright (c) 2020 Amélia O. F. da S.

int mmapi_create_device(char* path,mmapi_device **device)
{
    int ret;
    key_t key=ftok("shm",7);
    int shm=shmget(key,sizeof(mmapi_device),IPC_CREAT);
    (*device)=shmat(shm,(void*)0,0);
    if(!(*device))return NULL;
    (*device)->shm=NULL;
    (*device)->selfshm=shm;
    if((ret=mmapi_start(*device,path))<0)
    {
        shmdt(device);
    }
    return ret;
}

int mmapi_available_names(char** names,char** paths,int len,int name_size,int path_size)
{
    DIR *idev=opendir("/dev/input");
    struct dirent *entry;
    int fd;
    int i=0;
    char path[path_size];
    if(idev)
    {
        while(entry=readdir(idev))
        {
            if(entry->d_type==DT_CHR)
            {
                strncpy(path,"/dev/input/",path_size);
                strncat(path,entry->d_name,path_size);
                fd=open(path,O_RDONLY|O_NONBLOCK);
                if(fd<=0)continue;
                ioctl(fd,EVIOCGNAME(name_size),names[i]);
                if(names[i][0]==0)strcpy(names[i],"Unknown");
                strncpy(paths[i],path,path_size);
                if(++i==len)break;
            }
        }
        closedir(idev);
        return i;
    }
    else return MMAPI_E_PATH;
}

int mmapi_start(mmapi_device *device,char *path)
{
    if(device&&path)
    {
        if(pipe(device->ctl)==-1)
            return MMAPI_E_PIPE;
        device->fd=open(path, O_RDONLY|O_NONBLOCK);
        if(device->fd==-1)
        {
            _mmapi_close_ctl(device);
            return MMAPI_E_PATH;
        }
        ioctl(device->fd,EVIOCGNAME(sizeof(device->name)),device->name);
        return mmapi_start_thread(device);
    }
    else
        return MMAPI_E_NULLPOINTER;
}

int mmapi_start_thread(mmapi_device *device)
{
    int ppid=getpid();
    int f=fork();
    key_t key=ftok("shm",7);
    if(f>0)//Main thread
    {
        close(device->ctl[0]);//Close read
        return 0;
    }
    else if(f<0)
        return f;
    else
    {
        diewithparent(ppid)

        struct input_event evt;
        unsigned int command;
        mmapi_handler *curh;
        int nexth;
        close(device->ctl[1]);//Close write
        int shm=shmget(key,sizeof(mmapi_device),0);
        device=shmat(shm,(void*)0,0);
        while(1)
        {
            if(poll(&(struct pollfd){ .fd = device->ctl[0], .events = POLLIN }, 1, 0)==1)//If there is control data
            {
                command=read(device->ctl[0],(char*)command,sizeof(unsigned int));
                if(command&MMAPI_C_CLOSE)break;
            }
            if(read(device->fd,&evt,sizeof(evt))!=-1)
            {
                key_t key2=ftok("shm",10);
                if(device->shm)
                {
                    curh=shmat(device->shm,(void*)0,0);
                    if(curh->type&MMAPI_H_RAW)
                    {
                        write(curh->pipe[1],&evt,sizeof(evt));
                    }
                    if(curh->type&MMAPI_H_DEC)
                    {
                        write(curh->pipe[1],mmapi_decode(device,&evt),sizeof(mmapi_event));
                    }
                    nexth=curh->next;
                    while(nexth)
                    {
                        shmdt(curh);
                        curh=shmat(nexth,(void*)0,0);
                        if(curh->type&MMAPI_H_RAW)
                        {
                            write(curh->pipe[1],&evt,sizeof(evt));
                        }
                        if(curh->type&MMAPI_H_DEC)
                        {
                            write(curh->pipe[1],mmapi_decode(device,&evt),sizeof(mmapi_event));
                        }
                        nexth=curh->next;
                    }
                }
            }
        }
        return 0;
    }
}

mmapi_event mmapi_decode(mmapi_device *device,struct input_event *evt)
{
    int diff;//For tracking trackpad movements
    //These values were experimentally determined. Sorry.
    switch (evt->type)
    {
    case 1://Clicks
        switch(evt->code)
        {
        case 272://Left mouse button
            if(evt->value)
                return MMAPI_LCLICKDOWN;
            else return MMAPI_LCLICKUP;
            break;
        case 273://Right mouse button
            if(evt->value)
                return MMAPI_RCLICKDOWN;
            else return MMAPI_RCLICKUP;
            break;
        case 274://Middle mouse button
            if(evt->value)
                return MMAPI_MCLICKDOWN;
            else return MMAPI_MCLICKUP;
            break;
        case 330://Touchpad click start
            if(evt->value)
                return MMAPI_LCLICKDOWN;
            else return MMAPI_LCLICKUP;
            break;
        }
        break;
    case 2://Movement and Scrolling
        switch(evt->code)
        {
        case 0://Horizontal movement
            if(evt->value>0)
                return MMAPI_MOUSEMRIGHT;
            else if(evt->value<0)return MMAPI_MOUSEMLEFT;
            break;
        case 1://Vertical movement
            if(evt->value<0)
                return MMAPI_MOUSEMUP;
            else if(evt->value>0)return MMAPI_MOUSEMDOWN;
            break;
        case 8://Scrolling
            if(evt->value>0)
                return MMAPI_SCROLLUP;
            else return MMAPI_SCROLLDOWN;
        }
        break;
    case 3://Setting x/y (for trackpads only)
        switch (evt->code)//Bottom right edge seems to be the maximum value. Top left min.
        {
        case 53://x
            diff=evt->value-device->x;
            device->x=evt->value;
            if(diff>0) return MMAPI_MOUSEMRIGHT&MMAPI_UPDATEPOS;
            else if(diff<0) return MMAPI_MOUSEMLEFT&MMAPI_UPDATEPOS;
            else return MMAPI_UPDATEPOS;
            break;
        case 54://y
            diff=evt->value-device->y;
            device->y=evt->value;
            if(diff>0) return MMAPI_MOUSEMDOWN&MMAPI_UPDATEPOS;
            else if(diff<0) return MMAPI_MOUSEMUP&MMAPI_UPDATEPOS;
            else return MMAPI_UPDATEPOS;
            break;
        }
        break;
    }
    return 0;
}

int mmapi_free_device(mmapi_device *device)
{
    int shmid;
    if(device)
    {
        mmapi_free_handlers(device->shm);
        shmid=device->selfshm;
        shmdt(device);
        shmctl(shmid,IPC_RMID,0);
        device=NULL;
    }
}