/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <src/pyobjectconverter.hpp>
#include "converter_testwrap.hpp"

converterTest::converterTest()
{
}

converterTest::~converterTest()
{
}


PyObject* converterTest::TestObjectConversion(PyObject* obj)
{
  qi::GenericValuePtr res = GenericValue_from_PyObject(obj);

  return PyObject_from_GenericValue(res);
}
