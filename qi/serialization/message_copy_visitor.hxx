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
      : _signature(signature),
        _msgSrc(msgSrc),
        _msgDest(msgDest)
    {

    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::visit()
    {
      std::cout << "visit" << std::endl;

      qi::Signature::iterator it;

      for(it = _signature.begin(); it != _signature.end(); ++it)
      {
        visitElement(it);
      }
    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::visitElement(qi::Signature::iterator &elt)
    {
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
        break;
      default:
        onSimple(elt.signature);
        break;
      }
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
      qi::Signature           sig(elementType);
      qi::Signature::iterator it;
      it = sig.begin();

      for (i = 0; i < count; ++i)
      {
        visitElement(it);
      }
    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::onMap(const char *keyType, const char *valueType)
    {
      int count, i;
      _msgSrc.readInt(count);
      _msgDest.writeInt(count);
      qi::Signature           sigk(keyType);
      qi::Signature::iterator itk = sigk.begin();
      qi::Signature           sigv(valueType);
      qi::Signature::iterator itv = sigv.begin();

      for (i = 0; i < count; ++i)
      {
        visitElement(itk);
        visitElement(itv);
      }
    }

    template <typename MessageSrc, typename MessageDest>
    void MessageCopyVisitor<MessageSrc, MessageDest>::onProtobuf(const char *signature)
    {
      std::cout << "protobuf" << std::endl;
    }

  }
}
