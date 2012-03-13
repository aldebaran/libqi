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

namespace qi
{
  class Buffer;

  class QIMESSAGING_API FunctorParameters {
  public:
    explicit FunctorParameters(const qi::Buffer *buffer)
      : _buffer(buffer)
    {}

    inline const qi::Buffer *buffer() const { return _buffer; }

  private:
    const qi::Buffer *_buffer;
  };


  class QIMESSAGING_API FunctorResultBase {
  public:
    virtual ~FunctorResultBase()                    = 0;
    virtual void setValue(const qi::Buffer *buffer) = 0;
  };

  class QIMESSAGING_API FunctorResult {
  public:

    FunctorResult()
      : _p(),
        _error(0)
    {}

    virtual ~FunctorResult() {}

    virtual void setValue(const qi::Buffer *buffer) { _p->setValue(buffer); }

    inline int       &error()       { return _error; }
    inline const int &error() const { return _error; }

    virtual void setError(int err) { _error = err; }

    bool isValid() const { return _p ? true : false; }

    //could be changed using inheritance
  protected:
    boost::shared_ptr<FunctorResultBase> _p;

  private:
    int                                  _error;
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
