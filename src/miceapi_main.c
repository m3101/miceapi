#include "miceapi_main.h"
#include "miceapi_events.h"

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

void miceapi_bufncpy(void*dest,void*src,int n)
{
    if(!dest||!src||n==0)return;
    int i;
    for(i=0;i<n;i++)((char*)dest)[i]=((char*)src)[i];
}

int miceapi_deviceid=0;
int miceapi_create_device(char* path,miceapi_device **device)
{
    int ret=0;
    key_t key=ftok(".",miceapi_deviceid);
    int shm=shmget(key,sizeof(miceapi_device),IPC_CREAT);
    (*device)=shmat(shm,(void*)0,0);
    if(!(*device)||(*device)==(void*)~0)return errno&EINVAL?miceapi_E_SHM:miceapi_E_ACCESS|miceapi_E_SHM;
    (*device)->shm=0;
    (*device)->ashm=0;
    (*device)->hid=0;
    (*device)->ahid=0;
    (*device)->id=miceapi_deviceid++;
    (*device)->selfshm=shm;
    if((ret=miceapi_start(*device,path))<0)
    {
        shmdt(*device);
    }
    return ret;
}

int miceapi_available_names(char** names,char** paths,int len,int name_size,int path_size)
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
    else return miceapi_E_PATH;
}

int miceapi_start(miceapi_device *device,char *path)
{
    if(device&&path)
    {
        device->fd=open(path, O_RDONLY|O_NONBLOCK);
        if(device->fd==-1)
        {
            return errno==EACCES?miceapi_E_ACCESS|miceapi_E_PATH:miceapi_E_PATH;
        }
        ioctl(device->fd,EVIOCGNAME(sizeof(device->name)),device->name);
        return miceapi_start_thread(device);
    }
    else
        return miceapi_E_NULLPOINTER;
}

int miceapi_start_thread(miceapi_device *device)
{
    int ppid=getpid();
    int f;
    key_t key=ftok(".",device->id);

    int shm=shmget(key,sizeof(miceapi_device),0);
    device=shmat(shm,(void*)0,0);
    if(device==(void*)~0||device==(void*)0)return miceapi_E_SHM;

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
        miceapi_handler *curh;
        miceapi_advhandler *acurh;
        int nexth;
        while(device)
        {
            if(read(device->fd,&evt,sizeof(evt))!=-1)
            {
                if(evt.type==0)continue;
                if(device->shm)
                {
                    miceapi_event decoded=miceapi_decode(device,&evt);
                    if(decoded==0)continue;
                    curh=shmat(device->shm,(void*)0,0);
                    miceapi_bufncpy(&curh->buffer[(curh->ec++)*sizeof(miceapi_event)],(char*)&decoded,sizeof(miceapi_event));
                    curh->ec%=miceapi_H_BUFSIZ;
                    nexth=curh->next;
                    while(nexth)
                    {
                        shmdt(curh);
                        curh=shmat(nexth,(void*)0,0);
                        miceapi_bufncpy(&curh->buffer[(curh->ec++)*sizeof(miceapi_event)],(char*)&decoded,sizeof(miceapi_event));
                        curh->ec%=miceapi_H_BUFSIZ;
                        nexth=curh->next;
                    }
                    shmdt(curh);
                }
                if(device->ashm)
                {
                    acurh=shmat(device->ashm,(void*)0,0);
                    miceapi_bufncpy(&acurh->buffer[(acurh->ec++)*sizeof(struct input_event)],(char*)&evt,sizeof(struct input_event));
                    acurh->ec%=miceapi_H_BUFSIZ;
                    nexth=acurh->next;
                    while(nexth)
                    {
                        shmdt(acurh);
                        acurh=shmat(nexth,(void*)0,0);
                        miceapi_bufncpy(&acurh->buffer[(acurh->ec++)*sizeof(struct input_event)],(char*)&evt,sizeof(struct input_event));
                        acurh->ec%=miceapi_H_BUFSIZ;
                        nexth=acurh->next;
                    }
                    shmdt(acurh);
                }
            }
        }
        return 0;
    }
}
//little endian, last 16 bits are from code, 17th is set to 1 (miceapi_OTHER), next 15 are from value
//That is: event = [15bits-value|1bit-miceapi_OTHER flag|16bits-code](little endian)
#define _packevt(evt)    (evt->value<<(32-15))|\
                        miceapi_OTHER|\
                        (evt->code)
miceapi_event miceapi_decode(miceapi_device *device,struct input_event *evt)
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
                        return miceapi_LCLICKDOWN;
                    else return miceapi_LCLICKUP;
                    break;
                case 273://Right mouse button
                    if(evt->value)
                        return miceapi_RCLICKDOWN;
                    else return miceapi_RCLICKUP;
                    break;
                case 274://Middle mouse button
                    if(evt->value)
                        return miceapi_MCLICKDOWN;
                    else return miceapi_MCLICKUP;
                    break;
                case 330://Touchpad click start
                    if(evt->value)
                        return miceapi_LCLICKDOWN;
                    else return miceapi_LCLICKUP;
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
                    return miceapi_MOUSEMRIGHT;
                else if(evt->value<0)return miceapi_MOUSEMLEFT;
                break;
            case 1://Vertical movement
                if(evt->value<0)
                    return miceapi_MOUSEMUP;
                else if(evt->value>0)return miceapi_MOUSEMDOWN;
                break;
            case 8://Scrolling
                if(evt->value>0)
                    return miceapi_SCROLLUP;
                else return miceapi_SCROLLDOWN;
            }
            break;
        case 3://Setting x/y (for trackpads only)
            switch (evt->code)//Bottom right edge seems to be the maximum value. Top left min.
            {
            case 53://x
                diff=evt->value-device->x;
                device->x=evt->value;
                if(diff>0) return miceapi_MOUSEMRIGHT|miceapi_UPDATEPOS;
                else if(diff<0) return miceapi_MOUSEMLEFT|miceapi_UPDATEPOS;
                else return miceapi_UPDATEPOS;
                break;
            case 54://y
                diff=evt->value-device->y;
                device->y=evt->value;
                if(diff>0) return miceapi_MOUSEMDOWN|miceapi_UPDATEPOS;
                else if(diff<0) return miceapi_MOUSEMUP|miceapi_UPDATEPOS;
                else return miceapi_UPDATEPOS;
                break;
            }
            break;
    }
    return 0;
}

int miceapi_free_device(miceapi_device **device)
{
    int shmid;
    if(device&&*device)
    {
        miceapi_free_handlers((*device)->shm,(*device)->hid);
        miceapi_free_advhandlers((*device)->ashm,(*device)->ahid);
        shmid=(*device)->selfshm;
        shmdt(*device);
        shmctl(shmid,IPC_RMID,0);
        *device=NULL;
    }
    return 0;
}