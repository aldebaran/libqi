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

      while (true) {
        qi::SignatureLexer::Element elt;
        elt = _lexer.getNext();
        if (!visitElement(elt))
          break;
      }
    }

    template <typename MessageSrc, typename MessageDest>
    int MessageCopyVisitor<MessageSrc, MessageDest>::visitElement(qi::SignatureLexer::Element &elt)
    {
      std::cout << "visit" << std::endl;
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
      case 0:
        return 0;
      default:
        onSimple(elt.signature);
        break;
      }
      return 1;
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
      case 'd':
        double d;
        _msgSrc.readDouble(d);
        _msgDest.writeDouble(d);
        break;
      case 's':
        std::string s;
        _msgSrc.readString(s);
        _msgDest.writeString(s);
        break;

      }
    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::onList(const char *elementType)
    {
      int count, i;
      _msgSrc.readInt(count);
      _msgDest.writeInt(count);
      qi::SignatureLexer lex(elementType);
      qi::SignatureLexer::Element elt;

      elt = lex.getNext();
      for (i = 0; i < count; ++i)
      {
        visitElement(elt);
      }
    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::onMap(const char *keyType, const char *valueType)
    {
      int count, i;
      _msgSrc.readInt(count);
      _msgDest.writeInt(count);
      qi::SignatureLexer          lexk(keyType);
      qi::SignatureLexer::Element eltk = lexk.getNext();
      qi::SignatureLexer          lexv(valueType);
      qi::SignatureLexer::Element eltv = lexv.getNext();

      for (i = 0; i < count; ++i)
      {
        visitElement(eltk);
        visitElement(eltv);
      }
    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::onProtobuf(const char *signature)
    {
      std::cout << "protobuf" << std::endl;
    }

  }
}
