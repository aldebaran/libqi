#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _PYTHON_SRC_PYOBJECT_HPP_
#define _PYTHON_SRC_PYOBJECT_HPP_

#include <qitype/anyobject.hpp>
#include <boost/python.hpp>

namespace qi {
  namespace py {
    boost::python::object makePyQiObject(qi::AnyObject obj, const std::string &name = std::string());
    qi::AnyObject         makeQiAnyObject(boost::python::object obj);
    void export_pyobject();
    void leakPush(qi::AnyReference ref);
    typedef std::vector<qi::AnyReference> LeakFrame;
    typedef std::vector<LeakFrame > LeakStack;
    LeakStack& leakStack();
    class LeakBlock
    {
    public:
      LeakBlock();
      ~LeakBlock();
    };
  }
}


#endif  // _PYTHON_SRC_PYOBJECT_HPP_
