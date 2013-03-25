%module qipy

%typemap(out) char** {
    char** it = $1;

    if (!it) {
        $result = Py_None;
    } else {
        $result = PyList_New(0);
        while (*it)  {
            PyObject* o = PyString_FromString(*it);
            PyList_Append($result, o);
            it++;
        }
    }
}

%{
#include <qic/object.h>
#include <qic/session.h>
#include <qic/future.h>
#include <qic/application.h>
#include <qic/value.h>
#include <qic/servicedirectory.h>
#include <src/qipython.hpp>
%}

#define QIC_API

%include <qic/object.h>
%include <qic/session.h>
%include <qic/future.h>
%include <qic/application.h>
%include <qic/value.h>
%include <qic/servicedirectory.h>
%include <src/qipython.hpp>

