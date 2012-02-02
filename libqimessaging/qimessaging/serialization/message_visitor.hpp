#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef _QIMESSAGING_SERIALIZATION_MESSAGE_VISITOR_HPP_
#define _QIMESSAGING_SERIALIZATION_MESSAGE_VISITOR_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/serialization/message.hpp>
#include <qimessaging/serialization/datastream.hpp>
#include <qimessaging/signature/signature_iterator.hpp>

namespace qi {
  namespace serialization {

    class QIMESSAGING_API MessageVisitor
    {
    public:
      MessageVisitor(qi::DataStream &msg, const char *signature);
      virtual ~MessageVisitor() {;}

      void visit();

    protected:
      virtual void onSimple(const char *simpleType);
      virtual void onList(const char *elementType);
      virtual void onMap(const char *keyType, const char *valueType);
      virtual void onProtobuf(const char *name);

    protected:
      qi::Signature               _sig;
      qi::DataStream &_message;
    };

  }
}



#endif  // _QIMESSAGING_SERIALIZATION_MESSAGE_VISITOR_HPP_
