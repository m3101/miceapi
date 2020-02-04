#include "mmapi_main.h"
#include <stdio.h>

int mmapi_create_device(char* path,mmapi_device **device)
{
    int ret;
    (*device)=malloc(sizeof(mmapi_device));
    if(!(*device))return NULL;
    if((ret=mmapi_start(*device,path))<0)
    {
        free(*device);
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
        if(pipe(device->evt)==-1)
        {
            _mmapi_close_ctl(device);
            return MMAPI_E_PIPE;
        }
        device->fd=open(path, O_RDONLY|O_NONBLOCK);
        if(device->fd==-1)
        {
            _mmapi_close_ctl(device);
            _mmapi_close_evt(device);
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
    int f=fork();
    if(f>0)
    {
        close(device->ctl[0]);//Close read
        close(device->evt[1]);//Close write
        return 0;
    }
    else if(f<0)
        return f;
    else
    {
        struct input_event evt;
        unsigned int command;
        close(device->ctl[1]);//Close write
        close(device->evt[0]);//Close read
        while(1)
        {
            if(poll(&(struct pollfd){ .fd = device->ctl[0], .events = POLLIN }, 1, 0)==1)//If there is control data
            {
                command=read(device->ctl[0],(char*)command,sizeof(unsigned int));
                if(command&MMAPI_C_CLOSE)break;
            }
            if(read(device->fd,&evt,sizeof(evt))!=-1)
            {
                write(device->evt[1],&evt,sizeof(evt));
            }
        }
        return 0;
    }
}

mmapi_event mmapi_wait(mmapi_device *device)
{
    struct input_event evt;
    printf("Name: %s\n",device->name);
    read(device->evt[0],&evt,sizeof(evt));
    return 10U;
}