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

#include <qimessaging/datastream.hpp>

namespace qi
{

  class FunctorParameters {
  public:
    explicit FunctorParameters(qi::IODevice *buffer)
      : _datastream(buffer)
    {}

    inline qi::DataStream       &datastream()       { return _datastream; }
    inline const qi::DataStream &datastream() const { return _datastream; }

  private:
    qi::DataStream  _datastream;
  };

  class FunctorResult {
  public:
    explicit FunctorResult(qi::IODevice *buffer)
      : _datastream(buffer),
        _error(0)
    {}

    inline qi::DataStream       &datastream()       { return _datastream; }
    inline const qi::DataStream &datastream() const { return _datastream; }

    inline int       &error()       { return _error; }
    inline const int &error() const { return _error; }

    void setError(int err) { _error = err; }

  private:
    qi::DataStream  _datastream;
    int             _error;
  };

  /**
  * Generic functor class
  * \ingroup Functors
  */
  class Functor {
  public:
    virtual void call(qi::FunctorParameters &params, qi::FunctorResult& result) const = 0;
    virtual ~Functor() {}
  };


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

}

#include <qimessaging/details/functor.hxx>
#include <qimessaging/details/voidfunctor.hxx>
#include <qimessaging/details/memberfunctor.hxx>
#include <qimessaging/details/voidmemberfunctor.hxx>

#endif  // _QIMESSAGING_FUNCTOR_HPP_
