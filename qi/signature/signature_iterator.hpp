/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_SIGNATURE_SIGNATURE_LEXER_HPP_
#define _QI_SIGNATURE_SIGNATURE_LEXER_HPP_

#include <exception>

namespace qi {



  /// Signature container. This class provide a useful iterator.
  /// \ingroup Signature
  /// \include example_qi_signature_iterator.cpp
  class Signature {
  public:
    Signature(const char *signature)
      : _signature(signature)
    {
      _end._signature = _signature;
      //+1 to be after the end
      _end._current   = _signature + strlen(_signature) + 1;
    }

    Signature(const std::string &signature)
      : _signature(signature.c_str())
    {
      _end._signature = _signature;
      //+1 to be after the end
      _end._current   = _signature + signature.size() + 1;
    }

    enum Type {
      None     = 0,
      Bool     = 'b',
      Char     = 'c',
      Void     = 'v',
      Int      = 'i',
      Float    = 'f',
      Double   = 'd',
      String   = 's',
      List     = '[',
      Map      = '{',
      Protobuf = '@'
    };

    /// Bad signature format
    class BadFormatException : public std::exception {
    public:
      BadFormatException () {}
      /// Constructor
      /// <param name="message">The message.</param>
      BadFormatException (const std::string & message) { this->message = message; }
      virtual ~BadFormatException () throw () {}
      /// <summary>Gets the exception message. </summary>
      /// <returns>The message</returns>
      virtual const char * what () const throw () { return message.c_str(); }
    private:
      std::string message;
    };



    /// <summary>
    /// Signature iterator, this will return type composing a signature one by one.
    /// </summary>
    class iterator {
    public:
      friend class qi::Signature;
      iterator &operator++() {
        return next();
      }

      bool operator!=(const iterator &rhs) {
        return (_signature != rhs._signature) || (_current != rhs._current);
      };

      bool operator==(const iterator &rhs) {
        return (_signature == rhs._signature) && (_current == rhs._current);
      };

      iterator &next();

      /// <summary> return the type of the current element. </summary>
      Type type()const {
        return static_cast<Type>(raw_signature[0]);
      }

      /// <summary> return the complete signature for the current type.
      /// This will include items types for list and map </summary>
      std::string signature()const {
        return std::string(raw_signature, _current - raw_signature);
      }

      /// <summary> return the item type for list, and the key type for map. return empty when the current
      /// type is not a list nor a map.
      /// </summary>
      std::string child_1()const {
        if (raw_child_1 && _child_1_current)
          return std::string(raw_child_1, _child_1_current - raw_child_1);
        return std::string();
      }

      /// <summary> return the value type for a map. return empty when the current type is not a a map.
      /// </summary>
      std::string child_2()const {
        if (raw_child_2 && _child_2_current)
          return std::string(raw_child_2, _child_2_current - raw_child_2);
        return std::string();
      }

    public:
      /// <summary> the raw signature, this is not a null terminated string, just a pointer to
      /// the current position in the signature buffer.
      /// </summary>
      const char         *raw_signature;

      /// <summary> true if the current type is a pointer </summary>
      bool                pointer;

      /// <summary> the raw child1, this is not a null terminated string, just a pointer to
      /// the current position in the signature buffer.
      /// raw_child_1 is null for all value except list and map. For list it represent point to the
      /// the start of items type of the list. For the map it point to the start of the key type of the map.
      /// </summary>

      const char         *raw_child_1;
      /// <summary> the raw child2, this is not a null terminated string, just a pointer to
      /// the current position in the signature buffer.
      /// raw_child_1 is null for all value except map. For the map it point to the start of the value type of the map.
      /// </summary>
      const char         *raw_child_2;

    private:
      void eat() { _current++; }
      const char *_signature;
      const char *_current;
      const char *_child_1_current;
      const char *_child_2_current;
    };



    iterator begin()const {
      iterator it;
      it._signature = _signature;
      it._current   = _signature;
      it.next();
      return it;
    }

    const iterator &end()const {
      return _end;
    }

  private:
    iterator    _end;
    const char *_signature;
  };

}

#endif  // _QI_SIGNATURE_SIGNATURE_LEXER_HPP_
