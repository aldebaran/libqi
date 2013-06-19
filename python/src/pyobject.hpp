/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/


#ifndef   	PYOBJECT_HPP_
# define   	PYOBJECT_HPP_

#include <qitype/genericobject.hpp>
#include <boost/python.hpp>

namespace qi {
  namespace py {
    boost::python::object makePyQiObject(qi::AnyObject obj, const std::string &name = std::string());
    qi::AnyObject         makeQiAnyObject(boost::python::object obj);
    void export_pyobject();
  }
}


#endif	    /* !PYOBJECT_HPP_ */
