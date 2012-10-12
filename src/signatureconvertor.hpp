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
    enum SignatureType {
      STL,
      Qt
    };

    SignatureConvertor(const qi::Signature *sig, SignatureType type = STL, bool constify = true);

    const std::string &signature();

  protected:
    void visit(const Signature *sig);
    void visitSingle(qi::Signature::iterator *it, bool constify);
    void visitSimple(qi::Signature::iterator *it, bool constify);
    void visitList(qi::Signature::iterator *it, bool constify);
    void visitMap(qi::Signature::iterator *it, bool constify);
    void visitTuple(qi::Signature::iterator *it, bool constify);

    const char *elementTypeSTL(int idx, bool constify) const;

    SignatureType  _type;
    bool           _done;
    bool           _constify;
    const qi::Signature *_sig;
    std::string    _result;
  };

};


#endif  // _SRC_SIGNATURECONVERTOR_HPP_
