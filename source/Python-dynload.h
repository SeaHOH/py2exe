/*
  This module allows us to dynamically load the python DLL.

  We have to #define Py_BUILD_CORE when we cmpile our stuff,
  then the exe doesn't try to link with pythonXY.lib, and also
  the following definitions compile.

  We use MyGetProcAddress to get the functions from the dynamically
  loaded python DLL, so it will work both with the DLL loaded from the
  file system as well as loaded from memory.

  Problems:
  - We cannot use vararg functions that have no va_list counterpart.
  - What about the flags or other data exported from Python?
  - Error handling MUST be improved...
  - Should we use a python script to generate this code
    from function prototypes automatically?
*/

#define Py_BUILD_CORE_BUILTIN
#include <Python.h>

#if !defined(__CYGWIN__)
#define PyAPI_FUNC(RTYPE) __declspec(dllimport) RTYPE
#endif
#define PyAPI_DATA(RTYPE) extern __declspec(dllimport) RTYPE
#ifdef STANDALONE
#define PyMODINIT_FUNC __declspec(dllexport) PyObject*
#endif

#include <marshal.h>

#if (PY_VERSION_HEX >= 0x030C0000) && defined(STANDALONE)

#include <windows.h>
#include "MyLoadLibrary.h"

/*
  The python dll may be loaded from memory or in the usual way.
  MyGetProcAddress handles both cases.
*/

#define DL_FUNC(name) (FARPROC)name = MyGetProcAddress(hmod_pydll, #name)
#define DL_DATA_PTR(name, myname) \
        (FARPROC)myname = MyGetProcAddress(hmod_pydll, #name)

////////////////////////////////////////////////////////////////

int
(*_PyImport_CheckSubinterpIncompatibleExtensionAllowed)(const char *name);

////////////////////////////////////////////////////////////////

void
dynload_pydll(void)
{
    PyObject *pmodname = PyUnicode_FromString("sys");
    PyObject *pattrname = PyUnicode_FromString("dllhandle");
    PyObject *sys = PyImport_Import(pmodname);
    PyObject *dllhandle = PyObject_GetAttr(sys, pattrname);
    HMODULE hmod_pydll = (HMODULE)PyLong_AsVoidPtr(dllhandle);
    Py_DECREF(pattrname);
    Py_DECREF(pmodname);
    Py_DECREF(sys);
    Py_DECREF(dllhandle);

    DL_FUNC(_PyImport_CheckSubinterpIncompatibleExtensionAllowed);
}

#endif
