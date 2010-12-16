/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

namespace qi {
  namespace serialization {



    template <typename MessageSrc, typename MessageDest>
    MessageCopyVisitor<MessageSrc, MessageDest>::MessageCopyVisitor(MessageSrc &msgSrc, MessageDest &msgDest, const char *signature)
      : _lexer(signature),
        _msgSrc(msgSrc),
        _msgDest(msgDest)
    {

    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::visit()
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

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::onSimple(const char *simpleType)
    {
      switch(*simpleType) {
      case 'b':
        bool b;
        _msgSrc.readBool(b);
        _msgDest.writeBool(b);
        break;
      case 'c':
        char c;
        _msgSrc.readChar(c);
        _msgDest.writeChar(c);
        break;
      case 'i':
        int i;
        _msgSrc.readInt(i);
        _msgDest.writeInt(i);
        break;
      case 'f':
        float f;
        _msgSrc.readFloat(f);
        _msgDest.writeFloat(f);
        break;
      case 's':
        std::string s;
        _msgSrc.readString(s);
        _msgDest.writeString(s);
        break;

      }

      std::cout << "simple type:" << (char) simpleType[0] << std::endl;
    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::onList(const char *elementType)
    {
      int count, i;
      _msgSrc.readInt(count);
      _msgDest.writeInt(count);
      std::cout << "list with " << count << "elements." << std::endl;
      MessageCopyVisitor<MessageSrc, MessageDest> mcv(_msgSrc, _msgDest, elementType);
      for (i = 0; i < count; ++i)
      {
        mcv.visit();
      }
    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::onMap(const char *keyType, const char *valueType)
    {
      int count, i;
      _msgSrc.readInt(count);
      _msgDest.writeInt(count);
      std::cout << "copy map with " << count << "elements." << std::endl;
      MessageCopyVisitor<MessageSrc, MessageDest> mv1(_msgSrc, _msgDest, keyType);
      MessageCopyVisitor<MessageSrc, MessageDest> mv2(_msgSrc, _msgDest, valueType);
      for (i = 0; i < count; ++i)
      {
        mv1.visit();
        mv2.visit();
      }
    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::onProtobuf(const char *signature)
    {
      std::cout << "protobuf" << std::endl;
    }

  }
}
