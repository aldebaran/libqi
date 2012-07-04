/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_FUNCTOR_HPP_
#define _QIMESSAGING_FUNCTOR_HPP_

// Activate or disactivate log
// #define QI_FUNCTOR_DEBUG(...) printf(__VA_ARGS__)
#define QI_FUNCTOR_DEBUG(...)

#include <qi/shared_ptr.hpp>
#include <qimessaging/api.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/buffer.hpp>

namespace qi
{

  class QIMESSAGING_API FunctorParameters {
  public:
    explicit FunctorParameters(const qi::Buffer &buffer)
      : _buffer(buffer)
    {}

    inline const qi::Buffer &buffer() const { return _buffer; }

  private:
    qi::Buffer _buffer;
  };


  class QIMESSAGING_API FunctorResultBase {
  public:
    virtual ~FunctorResultBase()                    = 0;
    virtual void setValue(const qi::Buffer &buffer) = 0;
    virtual void setError(const qi::Buffer &msg)    = 0;
  };

  class QIMESSAGING_API FunctorResult {
  public:

    FunctorResult()
      : _p()
    {}

    explicit FunctorResult(boost::shared_ptr<FunctorResultBase> p)
     : _p(p)
    {}

    virtual ~FunctorResult() {}

    virtual void setValue(const qi::Buffer &buffer) { _p->setValue(buffer); }
    virtual void setError(const qi::Buffer &msg)    { _p->setError(msg); }

    bool isValid() const { return _p ? true : false; }

    //could be changed using inheritance
  protected:
    boost::shared_ptr<FunctorResultBase> _p;
  };

  /**
  * Generic functor class
  * \ingroup Functors
  */
  class QIMESSAGING_API Functor {
  public:
    virtual void call(const qi::FunctorParameters &params, qi::FunctorResult result) const = 0;
    virtual ~Functor() {}
  };

  namespace detail { template <typename T> class FutureFunctorResult; }
  template <typename T>
  void makeFunctorResult(qi::FunctorResult *promise, qi::Future<T> *future)
  {
    qi::detail::FutureFunctorResult<T> fr(future);
    *promise = fr;
  }

  class DataStream;
  namespace detail {
    //return true if everything is correct
    QIMESSAGING_API bool sanityCheckAndReport(qi::DataStream &ds, qi::FunctorResult &fr);
  }

}


#ifdef DEBUG
//TODO
# define QI_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)
//# define QI_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)
//  if(params.size() != nbrparamsrequired)
//  {
//    throw ALERROR("ALFunctor", "call", std::string( "Wrong number of parameters for this call requiring: " #nbrparamsrequired
//                  " and received: " ) + std::string(atoi( params.size())) + " (total params is: " + params.toString( VerbosityMini ) );
//  }
#else
# define QI_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)
#endif

#include <qimessaging/details/functorresult.hxx>
#include <qimessaging/details/functor.hxx>
#include <qimessaging/details/voidfunctor.hxx>
#include <qimessaging/details/memberfunctor.hxx>
#include <qimessaging/details/voidmemberfunctor.hxx>

#endif  // _QIMESSAGING_FUNCTOR_HPP_
