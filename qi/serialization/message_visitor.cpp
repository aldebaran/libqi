/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/
#include <qi/serialization/message_visitor.hpp>

namespace qi {
  namespace serialization {



    MessageVisitor::MessageVisitor(Message &msg, const char *signature)
      : _lexer(signature),
        _message(msg)
    {

    }

    void MessageVisitor::visit()
    {
      std::cout << "visit" << std::endl;
      qi::SignatureLexer::Element elt;

      while (true) {
        elt = _lexer.getNext();
        switch (elt.signature[0]) {
        case '[':
          onList(elt.child_1);
          break;
        case '{':
          onMap(elt.child_1, elt.child_2);
          break;
        case '@':
          onProtobuf(elt.signature);
          break;
        default:
          onSimple(elt.signature);
          break;
        }

        if (!elt.signature || !elt.signature[0])
          break;
      }
      //SignatureVisitor::visit();
    }

    void MessageVisitor::onSimple(const char *simpleType)
    {
      std::cout << "simple type:" << (char) simpleType[0] << std::endl;
    }

    void MessageVisitor::onList(const char *elementType)
    {
      std::cout << "list" << std::endl;
    }

    void MessageVisitor::onMap(const char *keyType, const char *valueType)
    {
      std::cout << "map" << std::endl;
    }

    void MessageVisitor::onProtobuf(const char *signature)
    {
      std::cout << "protobuf" << std::endl;
    }

  }
}
