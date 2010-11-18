/**
* @author Aldebaran Robotics
* Aldebaran Robotics (c) 2007 All Rights Reserved - This file is confidential.\n
*
* Version : $Id$
*/

#ifndef   QI_FUNCTORS_FUNCTOR_HPP_
# define  QI_FUNCTORS_FUNCTOR_HPP_

// Activate or disactivate log
// #define QI_FUNCTOR_DEBUG(...) printf(__VA_ARGS__)
#define QI_FUNCTOR_DEBUG(...)

#include <qi/serialization/serializeddata.hpp>

namespace qi
{
  /**
  * Generic functor class
  */
  class Functor {
  public:
    virtual void call(qi::serialization::SerializedData &params, qi::serialization::SerializedData& result)const = 0;
    virtual ~Functor() {}
  };


#ifdef DEBUG
//TODO
# define QI_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)
//# define QI_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)
//  if(params.size() != nbrparamsrequired)
//  {
//    throw ALERROR("ALFunctor", "call", std::string( "Wrong number of parameters for this call requiring: " #nbrparamsrequired " and received: " ) + std::string(atoi( params.size())) + " (total params is: " + params.toString( VerbosityMini ) );
//  }
#else
# define QI_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)
#endif

}

#define I_WANT_HXX
#include <qi/functors/detail/functor.hxx>
#include <qi/functors/detail/voidfunctor.hxx>
#include <qi/functors/detail/memberfunctor.hxx>
#include <qi/functors/detail/voidmemberfunctor.hxx>
#undef I_WANT_HXX

#endif
