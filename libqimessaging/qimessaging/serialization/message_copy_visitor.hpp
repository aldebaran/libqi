#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef _QIMESSAGING_SERIALIZATION_MESSAGE_COPY_VISITOR_HPP_
#define _QIMESSAGING_SERIALIZATION_MESSAGE_COPY_VISITOR_HPP_

#include <qimessaging/message.hpp>
#include <qimessaging/signature/signature_iterator.hpp>

namespace qi {
  namespace serialization {

    /// <summary> Copy a message to another. This allow for conversion between arbitrary type of message.
    /// this class is templated with the two message type, this avoid having (slow) virtual for all function
    //// in message. (because they are really used too often). </summary>
    template <typename MessageSrc, typename MessageDest>
    class MessageCopyVisitor
    {
    public:
      MessageCopyVisitor(MessageSrc &msgSrc, MessageDest &msgDest, const char *signature);
      virtual ~MessageCopyVisitor() {;}

      void visit();

    protected:
      void visitElement(qi::Signature::iterator &elt);

    protected:
      void onSimple(const char *simpleType);
      void onList(const char *elementType);
      void onMap(const char *keyType, const char *valueType);
      void onProtobuf(const char *name);

    protected:
      qi::Signature       _signature;
      MessageSrc         &_msgSrc;
      MessageDest        &_msgDest;
    };

  }
}

#include <qimessaging/serialization/message_copy_visitor.hxx>

#endif  // _QIMESSAGING_SERIALIZATION_MESSAGE_COPY_VISITOR_HPP_
