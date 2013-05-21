/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/


#ifndef   	ERROR_HPP_
# define   	ERROR_HPP_

#include <qi/log.hpp>
#include <boost/python.hpp>

//this allow displaying error raised in the python world
#define PY_CATCH_ERROR(DO)                                  \
   try                                                      \
   {                                                        \
     DO;                                                    \
   }                                                        \
   catch (const boost::python::error_already_set &e)        \
   {                                                        \
     qiLogError("python") << PyFormatError();               \
   }


//http://stackoverflow.com/questions/1418015/how-to-get-python-exception-text
inline std::string PyFormatError()
{
  PyObject *exc,*val,*tb;
  boost::python::object formatted_list, formatted;
  PyErr_Fetch(&exc,&val,&tb);
  boost::python::handle<> hexc(exc),hval(boost::python::allow_null(val)),htb(boost::python::allow_null(tb));
  static boost::python::object traceback(boost::python::import("traceback"));
  //force exception only
  if (1 || !tb) {
    boost::python::object format_exception_only(traceback.attr("format_exception_only"));
    formatted_list = format_exception_only(hexc,hval);
  } else {
    boost::python::object format_exception(traceback.attr("format_exception"));
    formatted_list = format_exception(hexc,hval,htb);
  }
  formatted = boost::python::str("\n").join(formatted_list);
  return boost::python::extract<std::string>(formatted);
}



#endif	    /* !ERROR_HPP_ */
