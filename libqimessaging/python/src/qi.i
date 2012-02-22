%module qi

%{
#include <qimessaging/c/qi_c.h>
#include <src/qipython.hpp>
%}

#define QIMESSAGING_API

%include <qimessaging/c/qi_c.h>
%include <qimessaging/c/server_c.h>
%include <qimessaging/c/signature_c.h>
%include <qimessaging/c/message_c.h>
%include <qimessaging/c/session_c.h>
%include <src/qipython.hpp>
