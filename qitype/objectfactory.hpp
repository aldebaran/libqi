#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_OBJECTFACTORY_HPP_
#define _QIMESSAGING_OBJECTFACTORY_HPP_

#include <qitype/api.hpp>
#include <qitype/genericobject.hpp>

namespace qi {

  QITYPE_API bool registerObjectFactory(const std::string& name, boost::function<qi::ObjectPtr (const std::string&)> factory);

  QITYPE_API qi::ObjectPtr createObject(const std::string& name);

  /// Get all factory names. Order is guaranteed to be the registration order
  QITYPE_API std::vector<std::string> listObjectFactories();

  /// Load a shared library and return the list of new available factory names.
  QITYPE_API std::vector<std::string> loadObject(const std::string& name, int flags = -1);
}

#define QI_REGISTER_OBJECT_FACTORY(name, func) \
  static bool _register_factory_ ## __LINE__ = ::qi::registerObjectFactory(name, func)

#endif  // _QIMESSAGING_OBJECTFACTORY_HPP_
