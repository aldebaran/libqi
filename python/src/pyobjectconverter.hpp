/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef PYOBJECTCONVERTER_HPP_
# define PYOBJECTCONVERTER_HPP_

# include <Python.h>

# include <qitype/type.hpp>

PyObject* PyObject_from_GenericValue(qi::GenericValuePtr val);
void PyObject_from_GenericValue(qi::GenericValuePtr val, PyObject** target);
qi::GenericValuePtr GenericValue_from_PyObject(PyObject* val);

#endif // !PYOBJECTCONVERTER_HPP_
