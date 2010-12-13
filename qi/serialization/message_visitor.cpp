
#include <qi/serialization/message_visitor.hpp>

namespace qi {
  namespace serialization {



    MessageVisitor::MessageVisitor(Message &msg, const char *signature)
      : SignatureVisitor(signature),
        _message(msg)
    {

    }

    void MessageVisitor::visit()
    {
      std::cout << "visit" << std::endl;
      SignatureVisitor::visit();
    }

    void MessageVisitor::onFunction(const std::string &name, const char *prototype)
    {
      std::cout << "function:" << name << std::endl;
      //visit(prototype);
    }

    void MessageVisitor::onFunctionPrototype(const char *returnType, const char *argumentsType)
    {
      std::cout << "function proto" << std::endl;
    }

    void MessageVisitor::onSimple(const char *simpleType)
    {
      std::cout << "simple type:" << (char) simpleType[0] << std::endl;
    }

    void MessageVisitor::onList(const int count, const char *elementType)
    {
      std::cout << "list" << std::endl;
    }

    void MessageVisitor::onMap(const int count, const char *keyType, const char *valueType)
    {
      std::cout << "map" << std::endl;
    }

    void MessageVisitor::onProtobuf(const std::string &name)
    {
      std::cout << "protobuf" << std::endl;
    }

  }
}
//JsonMessageVisitor::MessageVisitor(const Message &msg)
//  : _message(msg)
//{

//}

//void JsonMessageVisitor::onFunction(const std::string &name, const SignatureType &prototype)
//{

//}

//void JsonMessageVisitor::onFunctionPrototype(const SignatureType &returnType, const SignatureType &argumentsType)
//{

//}

//void JsonMessageVisitor::onSimple(const SignatureType &simpleType)
//{

//}

//void JsonMessageVisitor::onList(const SignatureType &elementType) {
//  result += "[ ";
//  for (int i = 0; i < size; ++i) {
//    visit(keyType);
//    if (i != size - 1)
//      result += ", ";
//  }
//  result += "]";
//}

//void JsonMessageVisitor::onMap(const int size, const SignatureType keyType, SignatureType valueType) {
//  result += "{ ";
//  for (int i = 0; i < size; ++i) {
//    visit(keyType);
//    result += ":";
//    visit(valueType);
//  }
//  result += "}";
//}

//void MessageVisitor::onProtobuf(const std::string &name) {

//}


//virtual void onFunction(const std::string &name, const SignatureType &prototype);
//virtual void onFunctionPrototype(const SignatureType &returnType, const SignatureType &argumentsType);
//virtual void onSimple(const SignatureType &simpleType);
//virtual void onList(const SignatureType &elementType);
//virtual void onMap(const SignatureType keyType, SignatureType valueType);
//virtual void onProtobuf(const std::string &name);
