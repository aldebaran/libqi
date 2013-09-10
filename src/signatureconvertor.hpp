#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_SIGNATURECONVERTOR_HPP_
#define _SRC_SIGNATURECONVERTOR_HPP_

# include <string>
# include <qitype/signature.hpp>

namespace qi {



  class SignatureConvertor {
  public:

    SignatureConvertor(const qi::Signature *sig);

    const std::string &signature();

  protected:
    void visit(const SignatureVector& elements);
    void visit(const qi::Signature& sig);
    void visitSimple(const qi::Signature &sig);
    void visitList(const qi::Signature &sig);
    void visitMap(const qi::Signature &sig);
    void visitTuple(const qi::Signature &sig);

    bool                 _done;
    const qi::Signature *_sig;
    std::string          _result;
  };

};


#endif  // _SRC_SIGNATURECONVERTOR_HPP_
