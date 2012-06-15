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

  namespace detail {

    //return true on success
    bool sanityCheckAndReport(qi::DataStream &ds, qi::FunctorResult &fr) {
      if (ds.status() != qi::DataStream::Status_Ok) {
        qi::Buffer buf;
        qi::DataStream dso(buf);
        dso << "Serialization error: " << int(dso.status());
        fr.setError(buf);
        return false;
      }
      return true;
    }

  }
}

