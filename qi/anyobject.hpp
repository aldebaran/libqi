#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_ANYOBJECT_HPP_
#define _QI_ANYOBJECT_HPP_

#include <map>
#include <string>

#include <qi/atomic.hpp>
#include <qi/type/api.hpp>
#include <qi/signal.hpp>
#include <qi/eventloop.hpp>
#include <qi/type/typeobject.hpp>

namespace qi {

namespace detail
{
  typedef std::map<TypeInfo, boost::function<AnyReference(AnyObject)> >
    ProxyGeneratorMap;
  QITYPE_API ProxyGeneratorMap& proxyGeneratorMap();

  // Storage type used by Object<T>, and Proxy.
  typedef boost::shared_ptr<class GenericObject> ManagedObjectPtr;
}

//all methods ID lesser than this constant are considered special.
//they are reserved for internal use by qi/qitype/qimessaging.
//(see boundobject.cpp for details)
static const unsigned int qiObjectSpecialMemberMaxUid = 100;

/** Make a call honoring ThreadingModel requirements
 *
 * Check the following rules in order:
 * - If \p el is set, force call in it overriding all rules.
 * - If method type is not auto, honor it, overriding callType
 * - If callType is set (not auto), honor it.
 * - Be synchronous.
 *
 * @param callerId: thread id of caller, for tracing purposes
 *
 * When the call is finally made, if ObjectThreadingModel
 * is SingleThread, acquire the object lock.
 */
QITYPE_API qi::Future<AnyReference> metaCall(EventLoop* el,
    ObjectThreadingModel objectThreadingModel,
    MetaCallType methodThreadingModel,
    MetaCallType callType,
    AnyObject manageable,
    unsigned int methodId,
    AnyFunction func,
    const GenericFunctionParameters& params,
    bool noCloneFirst = false,
    unsigned int callerId = 0,
    qi::os::timeval postTimestamp = qi::os::timeval());

}

#include <qi/type/detail/genericobject.hpp>

#include <qi/type/detail/object.hxx>

#include <qi/type/detail/async.hxx>

#include <qi/type/detail/proxyregister.hpp>

#include <qi/type/objecttypebuilder.hpp>

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::MinMaxSum,
  ("minValue",       minValue),
  ("maxValue",       maxValue),
  ("cumulatedValue", cumulatedValue));

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::MethodStatistics,
  ("count",  count),
  ("wall",   wall),
  ("user",   user),
  ("system", system));

QI_TYPE_STRUCT_AGREGATE_CONSTRUCTOR(qi::EventTrace,
  ("id",            id),
  ("kind",          kind),
  ("slotId",        slotId),
  ("arguments",     arguments),
  ("timestamp",     timestamp),
  ("userUsTime",    userUsTime),
  ("systemUsTime",  systemUsTime),
  ("callerContext", callerContext),
  ("calleeContext", calleeContext));

QI_TYPE_STRUCT(qi::os::timeval, tv_sec, tv_usec);

#endif  // _QITYPE_ANYOBJECT_HPP_
