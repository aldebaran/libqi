#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _PYTHON_SRC_PYSIGNAL_HPP_
#define _PYTHON_SRC_PYSIGNAL_HPP_

#include <boost/python.hpp>
#include <qitype/typeinterface.hpp>
#include <qitype/metasignal.hpp>
#include <qitype/anyobject.hpp>

namespace qi {
  class SignalBase;
  namespace py {
    boost::python::object makePySignal(const std::string &signature = "m");
    boost::python::object makePyProxySignal(const qi::AnyObject &obj, const qi::MetaSignal &signal);
    qi::SignalBase *getSignal(boost::python::object obj);
    void export_pysignal();
  }
}

#endif  // _PYTHON_SRC_PYSIGNAL_HPP_
