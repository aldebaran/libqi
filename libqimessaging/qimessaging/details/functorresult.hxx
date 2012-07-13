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
        qi::IDataStream ds(result);
        //TODO: avoid the copy
        T v;
        ds >> v;
        if (ds.status() != qi::IDataStream::Status_Ok)
          _f.setError("Serialization error.");
        else
          _f.setValue(v);
      }

      virtual void setError(const std::string &signature,
                            const qi::Buffer &message)
      {
        qi::IDataStream ds(message);

        if (signature != "s") {
          std::stringstream ss;
          ss << "Can't report the correct error message because the error signature is :" << signature;
          _f.setError(ss.str());
          return;
        }
        std::string err;
        ds >> err;
        if (ds.status() != qi::IDataStream::Status_Ok)
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

      virtual void setError(const std::string &signature,
                            const qi::Buffer &message)
      {
        qi::IDataStream ds(message);

        if (signature != "s") {
          std::stringstream ss;
          ss << "Can't report the correct error message because the error signature is :" << signature;
          _f.setError(ss.str());
          return;
        }
        std::string err;
        ds >> err;
        if (ds.status() != qi::IDataStream::Status_Ok)
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
        // FIXME Maybe we can do better
        // save callback
        std::vector<std::pair<FutureInterface<T> *, void *> > tmpCallbacks = future->callbacks();
        *future = p->future();

        typename std::vector<std::pair<FutureInterface<T> *, void *> >::iterator it;
        for (it = tmpCallbacks.begin(); it != tmpCallbacks.end(); ++it)
          future->addCallbacks((*it).first, (*it).second);
        _p = p;
      };
    };

  }

}

#endif

