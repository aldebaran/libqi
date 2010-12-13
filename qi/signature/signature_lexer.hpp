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
  class BadSignatureException: public std::exception {
  };
  /// <summary>
  /// Bad signature format
  /// </summary>
  class BadFormatException : public std::exception {
  public:

    /// <summary>Default constructor. </summary>
    BadFormatException () {}

    /// <summary>Constructor. </summary>
    /// <param name="message">The message.</param>
    BadFormatException (const std::string & message) { this->message = message; }

    /// <summary>Finaliser. </summary>
    virtual ~BadFormatException () throw () {}

    /// <summary>Gets the exception message. </summary>
    /// <returns>The message</returns>
    virtual const char * what () const throw () { return message.c_str(); }

  private:
    /// <summary> The message </summary>
    std::string message;
  };

  /// <summary>
  /// Signature Lexer, this will return type composing a signature one by one.
  /// </summary>
  class SignatureLexer {
  public:
    SignatureLexer(const char *signature);

    struct Element {
      const char         *signature;
      bool                pointer;
      const char         *child_1;
      const char         *child_2;
    };

    /// <summary> return an element representing a type </summary>
    SignatureLexer::Element getNext();

  protected:
    void eat() { _current++; }

  protected:
    const char *_signature;
    const char *_current;
  };
}

#endif  // _QI_SIGNATURE_SIGNATURE_LEXER_HPP_
