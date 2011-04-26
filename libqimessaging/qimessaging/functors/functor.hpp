#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_FUNCTORS_FUNCTOR_HPP_
#define _QI_FUNCTORS_FUNCTOR_HPP_

// Activate or disactivate log
// #define QI_FUNCTOR_DEBUG(...) printf(__VA_ARGS__)
#define QI_FUNCTOR_DEBUG(...)

#include <qimessaging/serialization.hpp>

namespace qi
{
  /**
  * Generic functor class
  * \ingroup Functors
  */
  class Functor {
  public:
    virtual void call(qi::serialization::Message &params, qi::serialization::Message& result)const = 0;
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

#include <qimessaging/functors/detail/functor.hxx>
#include <qimessaging/functors/detail/voidfunctor.hxx>
#include <qimessaging/functors/detail/memberfunctor.hxx>
#include <qimessaging/functors/detail/voidmemberfunctor.hxx>

#endif  // _QI_FUNCTORS_FUNCTOR_HPP_
