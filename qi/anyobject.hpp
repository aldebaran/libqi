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
#include <qi/api.hpp>
#include <qi/signal.hpp>
#include <qi/detail/executioncontext.hpp>
#include <qi/type/typeobject.hpp>

namespace qi {

using AnyObject = Object<Empty>;

//all methods ID lesser than this constant are considered special.
//they are reserved for internal use by qi/qitype/qimessaging.
//(see boundobject.cpp for details)
static const unsigned int qiObjectSpecialMemberMaxUid = 100;

/** Make a call honoring ThreadingModel requirements
 *
 * Check the following rules in order:
 * - If methodThreadingModel is not auto, honor it, overriding callType
 * - If el is set, force call in it (synchronously if we are in it).
 * - If callType is set (not auto), honor it.
 * - Be synchronous.
 *
 * When the call is finally made, if objectThreadingModel is SingleThread,
 * acquire the object lock.
 *
 * @param ec context into which the call will be scheduled
 * @param objectThreadingModel the threading model of the called object
 * @param methodThreadingModel the threading model of the specific method
 * @param callType the requested threading model
 * @param manageable the object on which to make the call
 * @param methodId the method id of the object to call
 * @param func the function to call
 * @param params the arguments of the call
 * @param noCloneFirst whether the first argument of the call should be cloned
 * or not
 * @param callerId thread id of caller, for tracing purposes
 * @param postTimestamp the time when the call was requested
 */
QI_API qi::Future<AnyReference> metaCall(ExecutionContext* ec,
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

#include <qi/type/detail/futureadapter.hpp>

#include <qi/type/detail/genericobject.hpp>

#include <qi/type/detail/object.hxx>

#include <qi/type/detail/proxyregister.hpp>

#include <qi/type/objecttypebuilder.hpp>

#include <qi/type/detail/futureadapter.hxx>

#endif  // _QITYPE_ANYOBJECT_HPP_
