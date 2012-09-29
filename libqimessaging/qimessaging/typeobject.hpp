#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef  __TYPEOBJECT_HPP_
# define __TYPEOBJECT_HPP_

#include <qimessaging/metaobject.hpp>
#include <qimessaging/future.hpp>
#include <qimessaging/signal.hpp>

namespace qi {

  enum MetaCallType {
    MetaCallType_Auto   = 0,
    MetaCallType_Direct = 1,
    MetaCallType_Queued = 2,
  };

  /* We will have 3 implementations for 3 classes of C++ class:
   * - DynamicObject
   * - T where T inherits from Manageable
   * - T
   * If the class does not inherit from Manageable, some features will be lost:
   *   - Automatic signal disconnection on object destruction
   *   - EventLoop support. emit will use the default policy
   *
   *
   *  NOTE: no SignalBase accessor at this point, but the backend is such that it would be possible
   *   but if we do that, virtual emit/connect/disconnect must go away, as they could be bypassed
   *  ->RemoteObject, ALBridge will have to adapt
   *
   */

  class QIMESSAGING_API ObjectType: public Type
  {
  public:
    virtual const MetaObject& metaObject(void* instance) = 0;
    virtual qi::Future<GenericValue> metaCall(void* instance, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto)=0;
    virtual void metaEmit(void* instance, unsigned int signal, const GenericFunctionParameters& params)=0;
    virtual qi::Future<unsigned int> connect(void* instance, unsigned int event, const SignalSubscriber& subscriber)=0;
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual qi::Future<void> disconnect(void* instance, unsigned int linkId)=0;
    /// @return the manageable interface for this instance, or 0 if not available
    virtual Manageable* manageable(void* instance) = 0;
    /// @return parent types with associated poniter offset
    virtual const std::vector<std::pair<Type*, int> >& parentTypes() = 0;
    virtual Type::Kind kind() const { return Type::Object;}
    /// @return -1 if there is no inheritance, or the pointer offset
    int inherits(Type* other);
  };

}

#endif   /* !TYPEOBJECT_PP_ */
