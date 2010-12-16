#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef _QI_SERIALIZATION_MESSAGE_COPY_VISITOR_HPP_
#define _QI_SERIALIZATION_MESSAGE_COPY_VISITOR_HPP_

#include <qi/serialization/message.hpp>
#include <qi/signature/signature_lexer.hpp>

namespace qi {
  namespace serialization {

    template <typename MessageSrc, typename MessageDest>
    class MessageCopyVisitor
    {
    public:
      MessageCopyVisitor(MessageSrc &msgSrc, MessageDest &msgDest, const char *signature);
      virtual ~MessageCopyVisitor() {;}

      void visit();

    protected:
      virtual void onSimple(const char *simpleType);
      virtual void onList(const char *elementType);
      virtual void onMap(const char *keyType, const char *valueType);
      virtual void onProtobuf(const char *name);

    protected:
      qi::SignatureLexer  _lexer;
      MessageSrc         &_msgSrc;
      MessageDest        &_msgDest;
    };

  }
}

#include <qi/serialization/message_copy_visitor.hxx>

#endif  // _QI_SERIALIZATION_MESSAGE_VISITOR_HPP_
