/*
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_OBJECT_FACTORY_HPP_
#define _QIMESSAGING_OBJECT_FACTORY_HPP_

namespace qi {

  QIMESSAGING_API bool registerObjectFactory(const std::string& name,
    boost::function<qi::Object*(const std::string&)> factory);

  QIMESSAGING_API qi::Object* createObject(const std::string& name);

  /// Get all factory names. Order is guaranteed to be the registration order
  QIMESSAGING_API std::vector<std::string> listObjectFactories();

  /// Load a shared library and return the list of new available factory names.
  QIMESSAGING_API std::vector<std::string> loadObject(const std::string& name,
    int flags = -1);
}

#define QI_REGISTER_OBJECT_FACTORY(name, func) \
  static bool _register_factory_ ## __LINE__ = ::qi::registerObjectFactory(name, func)

#endif
