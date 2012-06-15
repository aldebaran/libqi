/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_DETAILS_FUNCTORRESULT_HXX_
#define _QIMESSAGING_DETAILS_FUNCTORRESULT_HXX_

#include <qimessaging/datastream.hpp>

namespace qi {

  class Buffer;
  namespace detail {


    template <typename T>
    class FunctorResultBase_Typed : public FunctorResultBase {
    public:
      FunctorResultBase_Typed() { }

      void setValue(const T &value) {
        _f._p->setValue(value);
      }

      virtual void setValue(const qi::Buffer &result)
      {
        qi::DataStream ds(result);
        //TODO: avoid the copy
        T v;
        ds >> v;
        if (ds.status() != qi::DataStream::Status_Ok)
          _f.setError("Serialization error.");
        else
          _f.setValue(v);
      }

      virtual void setError(const qi::Buffer &message)
      {
        qi::DataStream ds(message);

        std::string err;
        ds >> err;
        if (ds.status() != qi::DataStream::Status_Ok)
          qiLogVerbose("qimessaging.functorresult") << "Can't report error in FunctorResult.setError";
        _f.setError(err);
      }

      Future<T> future() { return _f.future(); }

    protected:
      Promise<T> _f;
    };

    template <>
    class FunctorResultBase_Typed<void> : public FunctorResultBase {
    public:
      FunctorResultBase_Typed() { }

      void setValue(const void *value) {
        _f.setValue(value);
      }
      virtual void setValue(const qi::Buffer &QI_UNUSED(result))
      {
        _f.setValue(0);
      }

      virtual void setError(const qi::Buffer &message)
      {
        qi::DataStream ds(message);

        std::string err;
        ds >> err;
        if (ds.status() != qi::DataStream::Status_Ok)
          qiLogVerbose("qimessaging.functorresult") << "Can't report error in FunctorResult.setError";
        _f.setError(err);
      }

      Future<void> future() { return _f.future(); }

    protected:
      Promise<void> _f;
    };

    template <typename T>
    class FutureFunctorResult : public FunctorResult {
    public:
      FutureFunctorResult(qi::Future<T> *future) {
        boost::shared_ptr< FunctorResultBase_Typed<T> > p(new FunctorResultBase_Typed<T>());
        *future = p->future();
        _p = p;
      };
    };

  }

}

#endif

