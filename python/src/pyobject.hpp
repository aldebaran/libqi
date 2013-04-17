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

    boost::python::object makePyQiObject(qi::ObjectPtr obj, const std::string &name = std::string());

    void export_pyobject();

  }
}


#endif	    /* !PYOBJECT_HPP_ */
