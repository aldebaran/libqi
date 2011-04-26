%module qi

%{
#include <qimessaging/qi.h>
#include <src/qipython.hpp>
%}

#define QIMESSAGING_API

%include <qimessaging/qi.h>
%include <qimessaging/client.h>
%include <qimessaging/server.h>
%include <qimessaging/context.h>
%include <qimessaging/signature.h>
%include <qimessaging/message.h>
%include <src/qipython.hpp>
