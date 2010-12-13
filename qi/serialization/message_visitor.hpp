#ifndef _QI_SERIALIZATION_MESSAGE_VISITOR_HPP_
#define _QI_SERIALIZATION_MESSAGE_VISITOR_HPP_


//we want JsonMessage, TextMessage
//we want to copy between message type to convert data
//we want a treeview for qigui that can display a message, or allow create one


//we want PimpL for module: we count instanciate a real module, or a proxied one

#include <qi/serialization/message.hpp>
#include <qi/signature/signature_lexer.hpp>

namespace qi {
  namespace serialization {

    class MessageVisitor
    {
    public:
      MessageVisitor(Message &msg, const char *signature);

      void visit();

    protected:
      virtual void onFunction(const std::string &name, const char *prototype);
      virtual void onFunctionPrototype(const char *returnType, const char *argumentsType);
      virtual void onSimple(const char *simpleType);
      virtual void onList(const int count, const char *elementType);
      virtual void onMap(const int count, const char *keyType, const char *valueType);
      virtual void onProtobuf(const std::string &name);

    protected:
      qi::SignatureLexer          _lexer;
      qi::serialization::Message &_message;
    };

  }
}



#endif  // _QI_SERIALIZATION_MESSAGE_VISITOR_HPP_
