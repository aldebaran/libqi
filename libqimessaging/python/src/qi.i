%module qi

%{
#include <qimessaging/qi.h>
#include <src/qipython.hpp>
%}

#define QIMESSAGING_API

%include <qimessaging/qi.h>
%include <qimessaging/server.h>
%include <qimessaging/signature.h>
%include <qimessaging/message.h>
%include <qimessaging/session.h>
%include <src/qipython.hpp>
