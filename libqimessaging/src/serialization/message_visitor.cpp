/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/
#include <qimessaging/serialization/message_visitor.hpp>

namespace qi {
  namespace serialization {



    MessageVisitor::MessageVisitor(qi::DataStream &msg, const char *signature)
      : _sig(signature),
        _message(msg)
    {

    }

    void MessageVisitor::visit()
    {
      qi::Signature::iterator it;

      for (it = _sig.begin(); it != _sig.end(); ++it)
      {
        std::cout << "visit" << std::endl;

        switch (it.type()) {
        case qi::Signature::List:
          onList(it.raw_child_1);
          break;
        case qi::Signature::Map:
          onMap(it.raw_child_1, it.raw_child_2);
          break;
        case qi::Signature::Protobuf:
          onProtobuf(it.raw_signature);
          break;
        case 0:
          return;
        default:
          onSimple(it.raw_signature);
          break;
        }
      }
      //SignatureVisitor::visit();
    }

    void MessageVisitor::onSimple(const char *simpleType)
    {
      std::cout << "simple type:" << (char) simpleType[0] << std::endl;
    }

    void MessageVisitor::onList(const char *elementType)
    {
      int count, i;
      _message >> count;
      std::cout << "list with " << count << "elements." << std::endl;
      MessageVisitor mv(_message, elementType);
      for (i = 0; i < count; ++i)
      {
        mv.visit();
      }
    }

    void MessageVisitor::onMap(const char *keyType, const char *valueType)
    {
      int count, i;
      _message >> count;
      std::cout << "map with " << count << "elements." << std::endl;
      MessageVisitor mv1(_message, keyType);
      MessageVisitor mv2(_message, valueType);
      for (i = 0; i < count; ++i)
      {
        mv1.visit();
        mv2.visit();
      }
    }

    void MessageVisitor::onProtobuf(const char *signature)
    {
      std::cout << "protobuf" << std::endl;
    }

  }
}
