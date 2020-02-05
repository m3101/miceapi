#include "mmapi_main.h"
#include "mmapi_events.h"

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

void mmapi_bufncpy(void*dest,void*src,int n)
{
    if(!dest||!src||n==0)return;
    int i;
    for(i=0;i<n;i++)((char*)dest)[i]=((char*)src)[i];
}

int mmapi_deviceid=0;
int mmapi_create_device(char* path,mmapi_device **device)
{
    int ret=0;
    key_t key=ftok(".",mmapi_deviceid);
    int shm=shmget(key,sizeof(mmapi_device),IPC_CREAT);
    (*device)=shmat(shm,(void*)0,0);
    if(!(*device)||(*device)==(void*)~0)return errno&EINVAL?MMAPI_E_SHM:MMAPI_E_ACCESS|MMAPI_E_SHM;
    (*device)->shm=0;
    (*device)->ashm=0;
    (*device)->hid=0;
    (*device)->ahid=0;
    (*device)->id=mmapi_deviceid++;
    (*device)->selfshm=shm;
    if((ret=mmapi_start(*device,path))<0)
    {
        shmdt(*device);
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
        while((entry=readdir(idev)))
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
        device->fd=open(path, O_RDONLY|O_NONBLOCK);
        if(device->fd==-1)
        {
            return errno==EACCES?MMAPI_E_ACCESS|MMAPI_E_PATH:MMAPI_E_PATH;
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
    int f;
    key_t key=ftok(".",device->id);

    int shm=shmget(key,sizeof(mmapi_device),0);
    device=shmat(shm,(void*)0,0);
    if(device==(void*)~0||device==(void*)0)return MMAPI_E_SHM;

    f=fork();
    if(f>0)//Main thread
    {
        return 0;
    }
    else if(f<0)
        return f;
    else
    {
        diewithparent(ppid)

        struct input_event evt;
        mmapi_handler *curh;
        mmapi_advhandler *acurh;
        int nexth;
        while(device)
        {
            if(read(device->fd,&evt,sizeof(evt))!=-1)
            {
                if(evt.type==0)continue;
                if(device->shm)
                {
                    mmapi_event decoded=mmapi_decode(device,&evt);
                    if(decoded==0)continue;
                    curh=shmat(device->shm,(void*)0,0);
                    mmapi_bufncpy(&curh->buffer[(curh->ec++)*sizeof(mmapi_event)],(char*)&decoded,sizeof(mmapi_event));
                    curh->ec%=MMAPI_H_BUFSIZ;
                    nexth=curh->next;
                    while(nexth)
                    {
                        shmdt(curh);
                        curh=shmat(nexth,(void*)0,0);
                        mmapi_bufncpy(&curh->buffer[(curh->ec++)*sizeof(mmapi_event)],(char*)&decoded,sizeof(mmapi_event));
                        curh->ec%=MMAPI_H_BUFSIZ;
                        nexth=curh->next;
                    }
                    shmdt(curh);
                }
                if(device->ashm)
                {
                    acurh=shmat(device->ashm,(void*)0,0);
                    mmapi_bufncpy(&acurh->buffer[(acurh->ec++)*sizeof(struct input_event)],(char*)&evt,sizeof(struct input_event));
                    acurh->ec%=MMAPI_H_BUFSIZ;
                    nexth=acurh->next;
                    while(nexth)
                    {
                        shmdt(acurh);
                        acurh=shmat(nexth,(void*)0,0);
                        mmapi_bufncpy(&acurh->buffer[(acurh->ec++)*sizeof(struct input_event)],(char*)&evt,sizeof(struct input_event));
                        acurh->ec%=MMAPI_H_BUFSIZ;
                        nexth=acurh->next;
                    }
                    shmdt(acurh);
                }
            }
        }
        return 0;
    }
}
//little endian, last 16 bits are from code, 17th is set to 1 (MMAPI_OTHER), next 15 are from value
//That is: event = [15bits-value|1bit-MMAPI_OTHER flag|16bits-code](little endian)
#define _packevt(evt)    (evt->value<<(32-15))|\
                        MMAPI_OTHER|\
                        (evt->code)
mmapi_event mmapi_decode(mmapi_device *device,struct input_event *evt)
{
    int diff;//For tracking trackpad movements
    //These values were experimentally determined. Sorry.
    if(evt->type==0)return 0;
    switch (evt->type)
    {
        case 1://Clicks/keyboard events
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
                default://Keyboard event
                    return _packevt(evt);
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
                if(diff>0) return MMAPI_MOUSEMRIGHT|MMAPI_UPDATEPOS;
                else if(diff<0) return MMAPI_MOUSEMLEFT|MMAPI_UPDATEPOS;
                else return MMAPI_UPDATEPOS;
                break;
            case 54://y
                diff=evt->value-device->y;
                device->y=evt->value;
                if(diff>0) return MMAPI_MOUSEMDOWN|MMAPI_UPDATEPOS;
                else if(diff<0) return MMAPI_MOUSEMUP|MMAPI_UPDATEPOS;
                else return MMAPI_UPDATEPOS;
                break;
            }
            break;
    }
    return 0;
}

int mmapi_free_device(mmapi_device **device)
{
    int shmid;
    if(device&&*device)
    {
        mmapi_free_handlers((*device)->shm,(*device)->hid);
        mmapi_free_advhandlers((*device)->ashm,(*device)->ahid);
        shmid=(*device)->selfshm;
        shmdt(*device);
        shmctl(shmid,IPC_RMID,0);
        *device=NULL;
    }
    return 0;
}