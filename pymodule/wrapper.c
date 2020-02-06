#define PY_SSIZE_T_CLEAN
#include <python3.7m/Python.h>
#include "../src/miceapi_main.h"
#include "../src/miceapi_events.h"

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

/*
####miceapi.listDevices
    Returns a list of the first 64 devices' names and paths
    [(name,path),...]
*/
static PyObject *
miceapi_listDevices(PyObject *self, PyObject *args)
{
    char *paths[64];
    char *names[64];
    int devices,i,fd;
    PyListObject *ret;
    PyTupleObject *tmp;

    fd=open("/dev/input/event0",O_NONBLOCK|O_RDONLY);
    if(fd<=0)
    {
        PyErr_SetString(PyExc_OSError,"miceapi could not open device files at /dev/input. (Check access permissions)");
        return NULL;
    }
    close(fd);
    for(i=0;i<64;i++)
    {
        paths[i]=malloc(256*sizeof(char));
        names[i]=malloc(256*sizeof(char));
    }
    devices=miceapi_available_names(names,paths,64,256,256);
    ret=(PyListObject*)PyList_New((Py_ssize_t) devices);
    if(!ret)
    {
        PyErr_SetString(PyExc_MemoryError,"could not allocate new list object.");
        return NULL;
    }
    for(i=0;i<devices;i++)
    {
        tmp=(PyTupleObject*)PyTuple_New(2);
        if(!tmp)
        {
            PyErr_SetString(PyExc_MemoryError,"could not allocate new tuple object.");
            return NULL;
        }
        PyTuple_SetItem((PyObject*) tmp,0,_PyUnicode_FromASCII(names[i],strlen(names[i])));
        PyTuple_SetItem((PyObject*) tmp,1,_PyUnicode_FromASCII(paths[i],strlen(paths[i])));
        free(names[i]);
        free(paths[i]);
        PyList_SetItem((PyObject*) ret,i,(PyObject*) tmp);
    }
    return (PyObject*) ret;
}

/*
####miceapi.Device
    An abstraction of the C miceapi miceapi_device and miceapi_handler
*/

typedef struct {
    PyObject_HEAD
    miceapi_device *ob_devobj;
} deviceObject;

static void
device_dealloc(deviceObject *self)
{
    miceapi_free_device(&self->ob_devobj);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject*
device_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    deviceObject *self;
    self=(deviceObject*)type->tp_alloc(type,0);
    return (PyObject*)self;
}

static int
device_init(deviceObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"path" , NULL};
    char* path;
    int err;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist,
                                     &path))
    {
        PyErr_SetString(PyExc_SyntaxError,"miceapi.Device exiges a \"path\" argument");
        return -1;
    }
    if((err=miceapi_create_device(path,&self->ob_devobj)))
    {
        if(err&miceapi_E_PATH)
        {
            if(err&miceapi_E_ACCESS)
                PyErr_SetString(PyExc_SystemError,"miceapi could not access path (access forbidden).");
            else
                PyErr_SetString(PyExc_SyntaxError,"miceapi could not access path.");
        }
        else if(err&miceapi_E_SHM)
        {
            if(err&miceapi_E_ACCESS)
                PyErr_SetString(PyExc_SyntaxError,"miceapi wasn't authorized to access shared memory. Check permissions.");
            else
                PyErr_SetString(PyExc_SyntaxError,"Shared memory error. Zombie memory might be leftover from crash. Clear memory blocks with ipcs and ipcrm.\n");
        }
        return -1;
    }
    return 0;
}

/*
Device methods:
    (string)movement=device.wait_move()
        Synchronously waits for mouse movement and returns a string corresponding to the direction
        (up,down,left or right)
    (string)click=device.wait_mousedown(left=True,mid=False,right=False)
        Syncrhronously waits for a mousedown event on any of the buttons whose value was 'true' in the arguments
        If all buttons are false, it does nothing.
        Returns the button that was pressed (left,mid or right)
    (string)click=device.wait_mouseup(left=True,mid=False,right=False)
        Analogous to wait_mousedown, but for a mouseup event
    (string)dir=device.wait_scroll()
        Synchronously waits for scrolling. Returns the scroll direction (up or down)
*/

static PyObject *
device_wait_move(deviceObject *self, PyObject *args)
{
    if(!self->ob_devobj)
    {
        PyErr_SetString(PyExc_RuntimeError,"device object hasn't been properly instantiated");
        return NULL;
    }
    miceapi_handler *movehandler=miceapi_addhandler(self->ob_devobj);
    if(!movehandler)
    {
        PyErr_SetString(PyExc_RuntimeError,"couldn't attach handler object. Maybe shared memory is full?");
        return NULL;
    }
    miceapi_event evt=0;
    while(!(evt&miceapi_MOVEMENT))evt=miceapi_wait_handler(movehandler);
    miceapi_remove_handler(self->ob_devobj,movehandler->id);
    switch (evt&miceapi_MOVEMENT)
    {
        case miceapi_MOUSEMDOWN:
            return _PyUnicode_FromASCII("down",4);
            break;
        case miceapi_MOUSEMUP:
            return _PyUnicode_FromASCII("up",2);
            break;
        case miceapi_MOUSEMLEFT:
            return _PyUnicode_FromASCII("left",4);
            break;
        case miceapi_MOUSEMRIGHT:
            return _PyUnicode_FromASCII("right",5);
            break;
    }
    return _PyUnicode_FromASCII("unknown",7);
}

static PyObject *
device_wait_mousedown(deviceObject *self, PyObject *args, PyObject *kwds)
{
    int left=2,mid=2,right=2;
    static char *kwlist[] = {"left","mid","right", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ppp", kwlist,
                                     &left,&mid,&right))
        return NULL;
    if(left==2)left=1;
    if(mid==2)mid=0;
    if(right==2)right=0;

    if(left==0&&mid==0&&right==0)return _PyUnicode_FromASCII("unknown",7);

    if(!self->ob_devobj)
    {
        PyErr_SetString(PyExc_RuntimeError,"device object hasn't been properly instantiated");
        return NULL;
    }
    miceapi_handler *clickhandler=miceapi_addhandler(self->ob_devobj);
    if(!clickhandler)
    {
        PyErr_SetString(PyExc_RuntimeError,"couldn't attach handler object. Maybe shared memory is full?");
        return NULL;
    }
    miceapi_event evt=0;
    while(!(
            (left&&(evt&miceapi_LCLICKDOWN))||
            (mid&&(evt&miceapi_MCLICKDOWN))||
            (right&&(evt&miceapi_RCLICKDOWN))
        )){
        evt=miceapi_wait_handler(clickhandler);}
    miceapi_remove_handler(self->ob_devobj,clickhandler->id);
    switch (evt&miceapi_CLICKDOWN)
    {
    case miceapi_LCLICKDOWN:
        return _PyUnicode_FromASCII("left",4);
        break;
    case miceapi_MCLICKDOWN:
        return _PyUnicode_FromASCII("mid",3);
        break;
    case miceapi_RCLICKDOWN:
        return _PyUnicode_FromASCII("right",5);
        break;
    default:
        return _PyUnicode_FromASCII("unknown",7);
        break;
    }
}

static PyObject *
device_wait_mouseup(deviceObject *self, PyObject *args, PyObject *kwds)
{
    char left=2,mid=2,right=2;
    static char *kwlist[] = {"left","mid","right", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ppp", kwlist,
                                     &left,&mid,&right))
        return NULL;
    if(left==2)left=1;
    if(mid==2)mid=0;
    if(right==2)right=0;

    if(left==0&&mid==0&&right==0)return _PyUnicode_FromASCII("unknown",7);
    if(!self->ob_devobj)
    {
        PyErr_SetString(PyExc_RuntimeError,"device object hasn't been properly instantiated");
        return NULL;
    }
    miceapi_handler *clickhandler=miceapi_addhandler(self->ob_devobj);
    if(!clickhandler)
    {
        PyErr_SetString(PyExc_RuntimeError,"couldn't attach handler object. Maybe shared memory is full?");
        return NULL;
    }
    miceapi_event evt=0;
    while(!(
            (left&&(evt&miceapi_LCLICKUP))||
            (mid&&(evt&miceapi_MCLICKUP))||
            (right&&(evt&miceapi_RCLICKUP))
        ))
        evt=miceapi_wait_handler(clickhandler);
    miceapi_remove_handler(self->ob_devobj,clickhandler->id);
    switch (evt&miceapi_CLICKUP)
    {
    case miceapi_LCLICKUP:
        return _PyUnicode_FromASCII("left",4);
        break;
    case miceapi_MCLICKUP:
        return _PyUnicode_FromASCII("mid",3);
        break;
    case miceapi_RCLICKUP:
        return _PyUnicode_FromASCII("right",5);
        break;
    default:
        return _PyUnicode_FromASCII("unknown",7);
        break;
    }
}

static PyObject *
device_wait_scroll(deviceObject *self, PyObject *args)
{
    if(!self->ob_devobj)
    {
        PyErr_SetString(PyExc_RuntimeError,"device object hasn't been properly instantiated");
        return NULL;
    }
    miceapi_handler *scrollhandler=miceapi_addhandler(self->ob_devobj);
    if(!scrollhandler)
    {
        PyErr_SetString(PyExc_RuntimeError,"couldn't attach handler object. Maybe shared memory is full?");
        return NULL;
    }
    miceapi_event evt=0;
    while(!(evt&miceapi_SCROLL))evt=miceapi_wait_handler(scrollhandler);
    miceapi_remove_handler(self->ob_devobj,scrollhandler->id);
    switch (evt&miceapi_SCROLL)
    {
        case miceapi_SCROLLUP:
            return _PyUnicode_FromASCII("up",4);
            break;
        case miceapi_SCROLLDOWN:
            return _PyUnicode_FromASCII("down",2);
            break;
    }
    return _PyUnicode_FromASCII("unknown",7);
}

static PyMethodDef device_methods[] = {
    {"wait_move", (PyCFunction) device_wait_move, METH_NOARGS},
    {"wait_mousedown", (PyCFunction) device_wait_mousedown, METH_VARARGS|METH_KEYWORDS},
    {"wait_mouseup", (PyCFunction) device_wait_mouseup, METH_VARARGS|METH_KEYWORDS},
    {"wait_scroll", (PyCFunction) device_wait_scroll, METH_NOARGS},
    {NULL}
};

static PyTypeObject deviceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "miceapi.Device",
    .tp_doc = "miceapi device object",
    .tp_basicsize = sizeof(deviceObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = device_new,
    .tp_dealloc = (destructor) device_dealloc,
    .tp_init = (initproc) device_init,
    .tp_methods = device_methods,
};

/*
####miceapi
    Module definitions
*/

static PyMethodDef miceapiMethods[] = {
    {"listDevices",miceapi_listDevices,METH_NOARGS,
    "List available device names and paths (list of tuples (name,path))"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef miceapimodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "miceapi",
    .m_doc = "A module for managing multiple simultaneous pointer device input on linux environments.",
    .m_size = -1,
    .m_methods = miceapiMethods,
};

PyMODINIT_FUNC PyInit_miceapi(void)
{
    PyObject *m;
    if (PyType_Ready(&deviceType) < 0)
        return NULL;

    m = PyModule_Create(&miceapimodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&deviceType);
    if (PyModule_AddObject(m, "Device", (PyObject *) &deviceType) < 0) {
        Py_DECREF(&deviceType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}