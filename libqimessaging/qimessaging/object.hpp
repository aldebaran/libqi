/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	_QIMESSAGING_OBJECT_HPP_
# define   	_QIMESSAGING_OBJECT_HPP_

#include <map>
#include <string>

namespace qi {

  class MetaMethod {
  };

  class MetaObject {
    std::map<std::string, MetaMethod>   _methods;
    // std::map<std::string, MethodInfo>   _signals;
    // std::map<std::string, MethodInfo>   _slots;
    // std::map<std::string, PropertyInfo> _properties;
  };

  class Object {
  public:
    Object();
    virtual ~Object();

    MetaObject &metaObject();

    // void setProperty(const std::string &name, const qi::Value &value);
    // qi::Value getProperty(const std::string &name);

  protected:
    MetaObject *_meta;
  };
};

#endif
