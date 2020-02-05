#define PY_SSIZE_T_CLEAN
#include <python3.7m/Python.h>
#include "../src/mmapi_main.h"
#include "../src/mmapi_events.h"

//Copyright (c) 2020 Amélia O. F. da S.

/*
    mmapi.listDevices
    Returns a list of the first 64 devices' names and paths
    [(name,path),...]
*/
static PyObject *
mmapi_listDevices(PyObject *self, PyObject *args)
{
    char *paths[64];
    char *names[64];
    int devices,i,fd;
    PyListObject *ret;
    PyTupleObject *tmp;

    fd=open("/dev/input/event0",O_NONBLOCK|O_RDONLY);
    if(fd<=0)
    {
        PyErr_SetString(PyExc_OSError,"mmapi could not open device files at /dev/input. (Check access permissions)");
        return NULL;
    }
    close(fd);
    for(i=0;i<64;i++)
    {
        paths[i]=malloc(256*sizeof(char));
        names[i]=malloc(256*sizeof(char));
    }
    devices=mmapi_available_names(names,paths,64,256,256);
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

typedef struct {
    PyObject_HEAD
    mmapi_device *ob_devobj;
} deviceObject;

static void
device_dealloc(deviceObject *self)
{
    mmapi_free_device(&self->ob_devobj);
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
        PyErr_SetString(PyExc_SyntaxError,"mmapi.Device exiges a \"path\" argument");
        return -1;
    }
    if((err=mmapi_create_device(path,&self->ob_devobj)))
    {
        if(err&MMAPI_E_PATH)
        {
            if(err&MMAPI_E_ACCESS)
                PyErr_SetString(PyExc_SystemError,"mmapi could not access path (access forbidden).");
            else
                PyErr_SetString(PyExc_SyntaxError,"mmapi could not access path.");
        }
        else if(err&MMAPI_E_SHM)
        {
            if(err&MMAPI_E_ACCESS)
                PyErr_SetString(PyExc_SyntaxError,"mmapi wasn't authorized to access shared memory. Check permissions.");
            else
                PyErr_SetString(PyExc_SyntaxError,"Shared memory error. Zombie memory might be leftover from crash. Clear memory blocks with ipcs and ipcrm.\n");
        }
        return -1;
    }
    return 0;
}

static PyTypeObject deviceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mmapi.Device",
    .tp_doc = "MMAPI device objects",
    .tp_basicsize = sizeof(deviceObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = device_new,
    .tp_dealloc = (destructor) device_dealloc,
    .tp_init = (initproc) device_init,
};

static PyMethodDef mmapiMethods[] = {
    {"listDevices",mmapi_listDevices,METH_NOARGS,
    "List available device names and paths"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef mmapimodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "mmapi",
    .m_doc = "A module for managing multiple simultaneous pointer device input on linux environments.",
    .m_size = -1,
    .m_methods = mmapiMethods,
};

PyMODINIT_FUNC PyInit_mmapi(void)
{
    PyObject *m;
    if (PyType_Ready(&deviceType) < 0)
        return NULL;

    m = PyModule_Create(&mmapimodule);
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