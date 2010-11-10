/**
* @author Aldebaran Robotics
* Aldebaran Robotics (c) 2007 All Rights Reserved - This file is confidential.\n
*
* Version : $Id$
*/

#ifndef   QI_FUNCTORS_FUNCTOR_HPP_
# define  QI_FUNCTORS_FUNCTOR_HPP_

// Activate or disactivate log
// #define AL_FUNCTOR_DEBUG(...) printf(__VA_ARGS__)
#define AL_FUNCTOR_DEBUG(...)

#include <qi/messaging/call_definition.hpp>
#include <qi/messaging/result_definition.hpp>

namespace qi
{
  /**
  * Generic functor class
  */
  class Functor {
  public:
    virtual void call(const ::qi::Messaging::ArgumentList& pParams, ::qi::Messaging::ReturnValue& pResult) = 0;
    virtual ~Functor() {}
  };


#ifdef DEBUG
//TODO
# define AL_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)
//# define AL_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)
//  if(params.size() != nbrparamsrequired)
//  {
//    throw ALERROR("ALFunctor", "call", std::string( "Wrong number of parameters for this call requiring: " #nbrparamsrequired " and received: " ) + std::string(atoi( params.size())) + " (total params is: " + params.toString( VerbosityMini ) );
//  }
#else
# define AL_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)
#endif

}

#define I_WANT_HXX
#include <qi/functors/functor.hxx>
#include <qi/functors/voidfunctor.hxx>
#include <qi/functors/memberfunctor.hxx>
#include <qi/functors/voidmemberfunctor.hxx>
#undef I_WANT_HXX

#endif
