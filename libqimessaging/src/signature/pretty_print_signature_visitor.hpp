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

  namespace detail {

    class PrettyPrintSignatureVisitor {
    public:
      PrettyPrintSignatureVisitor(const char *signature, std::string &result);

      void visit(const char *sep = " ");

    protected:
      void visitSingle();
      void visitSimple();
      void visitList();
      void visitMap();
      void visitProtobuf();
      void visitTuple(bool param = false);
      bool visitFunction();

      std::string &_result;
      const char  *_current;
      const char  *_signature;
      std::string _method;
    };

  };

};


#endif  // _QIMESSAGING_SIGNATURE_DETAIL_PRETTY_PRINT_SIGNATURE_VISITOR_HPP_
