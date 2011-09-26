#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef _QI_SERIALIZATION_MESSAGE_VISITOR_HPP_
#define _QI_SERIALIZATION_MESSAGE_VISITOR_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/serialization/message.hpp>
#include <qimessaging/signature/signature_iterator.hpp>

namespace qi {
  namespace serialization {

    class QIMESSAGING_API MessageVisitor
    {
    public:
      MessageVisitor(Message &msg, const char *signature);
      virtual ~MessageVisitor() {;}

      void visit();

    protected:
      virtual void onSimple(const char *simpleType);
      virtual void onList(const char *elementType);
      virtual void onMap(const char *keyType, const char *valueType);
      virtual void onProtobuf(const char *name);

    protected:
      qi::Signature               _sig;
      qi::Message &_message;
    };

  }
}



#endif  // _QI_SERIALIZATION_MESSAGE_VISITOR_HPP_
