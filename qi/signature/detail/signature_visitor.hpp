/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_SIGNATURE_DETAIL_SIGNATURE_VISITOR_HPP__
#define   __QI_SIGNATURE_DETAIL_SIGNATURE_VISITOR_HPP__

# include<string>

namespace qi {

  namespace detail {

    class SignatureVisitor {
    public:
      SignatureVisitor(const char *signature, std::string &result);

      void visit(const char *sep = " ");

    protected:
      void visitSingle();
      void visitSimple();
      void visitList();
      void visitMap();
      void visitProtobuf();
      void visitFunctionArguments();

      std::string &_result;
      const char  *_signature;
      const char  *_current;
    };

  };

};


#endif // __QI_SIGNATURE_DETAIL_SIGNATURE_VISITOR_HPP__
