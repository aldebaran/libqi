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

  /// <summary>
  /// Bad signature format
  /// </summary>
  class BadFormatException : public std::exception {
  public:
    /// <summary>Constructor. </summary>
    BadFormatException () {}
    /// <summary>Constructor. </summary>
    /// <param name="message">The message.</param>
    BadFormatException (const std::string & message) { this->message = message; }
    virtual ~BadFormatException () throw () {}
    /// <summary>Gets the exception message. </summary>
    /// <returns>The message</returns>
    virtual const char * what () const throw () { return message.c_str(); }
  private:
    std::string message;
  };

  /// <summary> Signature container. This class is mostly useful because it implement an iterator </summary>
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

    public:
      const char         *signature;
      bool                pointer;
      const char         *child_1;
      const char         *child_2;

    protected:
      void eat() { _current++; }

      const char *_signature;
      const char *_current;
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

  protected:
    iterator    _end;
    const char *_signature;
  };

}

#endif  // _QI_SIGNATURE_SIGNATURE_LEXER_HPP_
