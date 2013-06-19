#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_TYPEOBJECT_HPP_
#define _QITYPE_TYPEOBJECT_HPP_

#include <qitype/metaobject.hpp>
#include <qi/future.hpp>
#include <qitype/functiontype.hpp>

namespace qi {

  /** Specifies how a call should be made.
   *  Can be used at both call-site, and callback-registration site.
   */
  enum MetaCallType {
    /// Honor the default behavior
    MetaCallType_Auto   = 0,
    /// Force a synchronous call
    MetaCallType_Direct = 1,
    /// Force an asynchronous call in an other thread
    MetaCallType_Queued = 2,
  };
  class SignalSubscriber;
  class Manageable;
  typedef qi::uint64_t Link;
  /* We will have 2 implementations for 2 classes of C++ class:
   * - DynamicObject: Use DynamicObjectBuilder
   * - T: Use ObjectTypeBuilder
   *
   * All values of this type (GenericObject) will be handled
   *
   *
   *  NOTE: no SignalBase accessor at this point, but the backend is such that it would be possible
   *   but if we do that, virtual emit/connect/disconnect must go away, as they could be bypassed
   *  ->RemoteObject, ALBridge will have to adapt
   *
   */

  class QITYPE_API ObjectTypeInterface: public TypeInterface
  {
  public:
    virtual const MetaObject& metaObject(void* instance) = 0;
    virtual qi::Future<AnyReference> metaCall(void* instance, Manageable* context, unsigned int method, const GenericFunctionParameters& params, MetaCallType callType = MetaCallType_Auto)=0;
    virtual void metaPost(void* instance, Manageable* context, unsigned int signal, const GenericFunctionParameters& params)=0;
    virtual qi::Future<Link> connect(void* instance, Manageable* context, unsigned int event, const SignalSubscriber& subscriber)=0;
    /// Disconnect an event link. Returns if disconnection was successful.
    virtual qi::Future<void> disconnect(void* instance, Manageable* context, Link linkId)=0;
    /// @return parent types with associated poniter offset
    virtual const std::vector<std::pair<TypeInterface*, int> >& parentTypes() = 0;
    virtual qi::Future<AnyValue> property(void* instance, unsigned int id) = 0;
    virtual qi::Future<void> setProperty(void* instance, unsigned int id, AnyValue value) = 0;
    virtual TypeKind kind() const { return TypeKind_Object;}
    /// @return -1 if there is no inheritance, or the pointer offset
    int inherits(TypeInterface* other);
  };

}

#endif  // _QITYPE_TYPEOBJECT_HPP_
