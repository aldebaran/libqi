/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/functor.hpp>

namespace qi {

  FunctorResultBase::~FunctorResultBase() {
  }

  void FunctorResult::setError(const std::string &str) {
    qi::Buffer buf;
    qi::ODataStream dso(buf);
    dso << str;
    _p->setError("s", buf);
  }


  namespace detail {

    //return true on success
    bool sanityCheckAndReport(qi::ODataStream &ds, qi::FunctorResult &fr) {
      if (ds.status() != qi::ODataStream::Status_Ok) {
        std::stringstream ss;
        ss << "Serialization error: " << int(ds.status());
        fr.setError(ss.str());
        return false;
      }
      return true;
    }
    bool sanityCheckAndReport(qi::IDataStream &ds, qi::FunctorResult &fr) {
      if (ds.status() != qi::IDataStream::Status_Ok) {
        std::stringstream ss;
        ss << "Serialization error: " << int(ds.status());
        fr.setError(ss.str());
        return false;
      }
      return true;
    }
  }
}

