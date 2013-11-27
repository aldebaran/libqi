#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_OBJECTFACTORY_HPP_
#define _QITYPE_OBJECTFACTORY_HPP_

#include <vector>

#include <boost/function.hpp>

#include <qitype/api.hpp>
#include <qitype/fwd.hpp>
#include <qitype/typeinterface.hpp>
#include <qitype/dynamicobjectbuilder.hpp>
#include <qitype/objecttypebuilder.hpp>

namespace qi {

  QITYPE_API bool registerObjectFactory(const std::string& name, boost::function<qi::AnyObject (const std::string&)> factory);

  QITYPE_API qi::AnyObject createObject(const std::string& name);

  /// Get all factory names. Order is guaranteed to be the registration order
  QITYPE_API std::vector<std::string> listObjectFactories();

  /// Load a shared library and return the list of new available factory names.
  QITYPE_API std::vector<std::string> loadObject(const std::string& name, int flags = -1);
}

/// register \p func as factory for object named \p name
#define QI_REGISTER_OBJECT_FACTORY(name, func) \
  static bool BOOST_PP_CAT(_register_factory_ ,__COUNTER__) = ::qi::registerObjectFactory(name, func)

/// register \p name's default constructor as factory for \p name
#define QI_REGISTER_OBJECT_FACTORY_CONSTRUCTOR(name) \
  QI_REGISTER_OBJECT_FACTORY(#name, boost::bind(::qi::detail::constructObject<name>))

/** Register a factory on \p name that creates an object with a single method
  * \p func named \p funcName
  */
#define QI_REGISTER_OBJECT_FACTORY_METHOD(name, funcName, func) \
  QI_REGISTER_OBJECT_FACTORY(name, ::qi::detail::makeObjectFactory(funcName, func))


/** Register an object as a factory for an other object
 * @param name target class name
 *
 * Will crate a factory for generated object 'name + "Service"',
 * with a create() method
 * that returns a newly created instance of \p name.
 */
#define QI_REGISTER_OBJECT_FACTORY_BUILDER(name)                      \
  QI_REGISTER_OBJECT_FACTORY_METHOD(#name "Service", "create", ::qi::detail::constructObject<name>)

#endif  // _QITYPE_OBJECTFACTORY_HPP_
