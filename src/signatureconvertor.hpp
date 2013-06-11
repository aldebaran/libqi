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
    void visit(const Signature *sig);
    void visitSingle(qi::Signature::iterator *it);
    void visitSimple(qi::Signature::iterator *it);
    void visitList(qi::Signature::iterator *it);
    void visitMap(qi::Signature::iterator *it);
    void visitTuple(qi::Signature::iterator *it);

    bool                 _done;
    const qi::Signature *_sig;
    std::string          _result;
  };

};


#endif  // _SRC_SIGNATURECONVERTOR_HPP_
