#define PY_SSIZE_T_CLEAN
#include <python3.7m/Python.h>
#include "../src/mmapi_main.h"

//Copyright (c) 2020 Amélia O. F. da S.

typedef struct {
    PyObject_HEAD
    mmapi_device ob_devobj;
} deviceObject;

static void
device_dealloc(deviceObject *self)
{
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject*
device_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    deviceObject *self;
    
}

static PyTypeObject deviceType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "mmapi.Device",
    .tp_doc = "MMAPI device objects",
    .tp_basicsize = sizeof(deviceObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = (destructor) device_dealloc,
};

static PyModuleDef mmapimodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "mmapi",
    .m_doc = "A module for managing multiple simultaneous pointer device input on linux environments.",
    .m_size = -1,
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