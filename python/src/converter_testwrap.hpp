/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef CONVERTER_TESTWRAP_HPP_
# define CONVERTER_TESTWRAP_HPP_

# include <Python.h>

class converterTest
{
  public:
    converterTest();
    ~converterTest();
    PyObject* TestObjectConversion(PyObject* obj);
};

#endif /* !CONVERTER_TESTWRAP_HPP_ */
