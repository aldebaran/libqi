#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _PYTHON_SRC_PYPROPERTY_HPP_
#define _PYTHON_SRC_PYPROPERTY_HPP_

#include <boost/python.hpp>
#include <qitype/metaproperty.hpp>
#include <qitype/anyobject.hpp>

namespace qi {
  class PropertyBase;
  namespace py {
    boost::python::object makePyProperty(const std::string &signature);
    qi::PropertyBase *getProperty(boost::python::object obj);
     boost::python::object makePyProxyProperty(const qi::AnyObject &obj, const qi::MetaProperty &prop);
    void export_pyproperty();
  }
}

#endif  // _PYTHON_SRC_PYPROPERTY_HPP_
