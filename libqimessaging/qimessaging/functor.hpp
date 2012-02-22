#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_FUNCTORS_FUNCTOR_HPP_
#define _QIMESSAGING_FUNCTORS_FUNCTOR_HPP_

// Activate or disactivate log
// #define QI_FUNCTOR_DEBUG(...) printf(__VA_ARGS__)
#define QI_FUNCTOR_DEBUG(...)

#include <qimessaging/datastream.hpp>

namespace qi
{
  /**
  * Generic functor class
  * \ingroup Functors
  */
  class Functor {
  public:
    virtual void call(qi::DataStream &params, qi::DataStream& result)const = 0;
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

#endif  // _QIMESSAGING_FUNCTORS_FUNCTOR_HPP_
