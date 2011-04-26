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

#include <qi/api.hpp>
#include <qi/serialization/message.hpp>
#include <qi/signature/signature_iterator.hpp>

namespace qi {
  namespace serialization {

    class QI_API MessageVisitor
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
      qi::serialization::Message &_message;
    };

  }
}



#endif  // _QI_SERIALIZATION_MESSAGE_VISITOR_HPP_
