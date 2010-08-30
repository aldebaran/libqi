/**
* @author Aldebaran Robotics
* Aldebaran Robotics (c) 2007 All Rights Reserved - This file is confidential.\n
*
* Version : $Id$
*/

#ifndef   AL_MESSAGING_FUNCTOR_HPP_
# define  AL_MESSAGING_FUNCTOR_HPP_

// Activate or disactivate log
// #define AL_FUNCTOR_DEBUG(...) printf(__VA_ARGS__)
#define AL_FUNCTOR_DEBUG(...)

#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/messaging/result_definition.hpp>

namespace AL
{

  /**
  * Generic functor class
  */
  class Functor {
  public:

    virtual void call(const ::AL::Messaging::CallDefinition& pParams, ::AL::Messaging::ResultDefinition& pResult) = 0;
    virtual ~Functor() {}
  };


#ifdef DEBUG
# define AL_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)         \
  if(params.getSize() != nbrparamsrequired)                             \
  {                                                                     \
    throw ALERROR("ALFunctor", "call", std::string( "Wrong number of parameters for this call requiring: " #nbrparamsrequired " and received: " ) + DecToString( params.getSize() ) + " (total params is: " + params.toString( VerbosityMini ) );                                    \
  }
#else
# define AL_FUNCTOR_ASSUME_NBR_PARAMS(params, nbrparamsrequired)
#endif

}

#define I_WANT_HXX
#include <alcommon-ng/functor/functor.hxx>
#include <alcommon-ng/functor/voidfunctor.hxx>
#include <alcommon-ng/functor/memberfunctor.hxx>
#include <alcommon-ng/functor/voidmemberfunctor.hxx>
#undef I_WANT_HXX

#endif
