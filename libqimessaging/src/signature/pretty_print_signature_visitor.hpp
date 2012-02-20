#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_SIGNATURE_DETAIL_PRETTY_PRINT_SIGNATURE_VISITOR_HPP_
#define _QIMESSAGING_SIGNATURE_DETAIL_PRETTY_PRINT_SIGNATURE_VISITOR_HPP_

# include<string>

namespace qi {



  class PrettyPrintSignatureVisitor {
  public:
    typedef enum {
      STL,
      Qt
    } SignatureType;

    enum {
      ErrorNone = 0,
      ErrorTrailingGarbage = 1,
    };

    PrettyPrintSignatureVisitor(const char *signature, SignatureType type = STL);


    const std::string &returnSignature();
    const std::string &functionSignature();

    int errno() const { return _errno; }

  protected:
    void visit();
    void visitSingle();
    void visitSimple();
    void visitList();
    void visitMap();
    void visitTuple();

    const char *elementTypeSTL(int idx);

    int           _errno;

    SignatureType _type;
    bool          _done;
    bool          _constify;

    std::string   _returnSig;
    std::string   _funcSig;

    const char   *_current;
    const char   *_signature;
    std::string   _method;
    std::string   _result;
  };

};


#endif  // _QIMESSAGING_SIGNATURE_DETAIL_PRETTY_PRINT_SIGNATURE_VISITOR_HPP_
