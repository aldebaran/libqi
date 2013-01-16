%module qimessagingswig

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
#include <qimessaging/c/qi_c.h>
#include <src/qipython.hpp>
#include <src/sd_testwrap.hpp>
#include <src/converter_testwrap.hpp>
#include <src/qipython_signal.hpp>
%}

#define QIMESSAGING_API

%include <qimessaging/c/qi_c.h>
%include <qimessaging/c/signature_c.h>
%include <qimessaging/c/message_c.h>
%include <qimessaging/c/object_c.h>
%include <qimessaging/c/session_c.h>
%include <qimessaging/c/future_c.h>
%include <qimessaging/c/application_c.h>
%include <src/qipython.hpp>
%include <src/sd_testwrap.hpp>
%include <src/converter_testwrap.hpp>

// Avoid generating a different wrapper for every default argument
%feature("compactdefaultargs") qi_signal::trigger;
%include <src/qipython_signal.hpp>
