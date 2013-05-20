/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/


#ifndef   	GIL_HPP_
# define   	GIL_HPP_

#include <qi/log.hpp>
#include <boost/python.hpp>

//this allow displaying error raised in the python world
#define PY_DISPLAY_ERROR(DO)                                \
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
  boost::python::object traceback(boost::python::import("traceback"));
  if (!tb) {
    boost::python::object format_exception_only(traceback.attr("format_exception_only"));
    formatted_list = format_exception_only(hexc,hval);
  } else {
    boost::python::object format_exception(traceback.attr("format_exception"));
    formatted_list = format_exception(hexc,hval,htb);
  }
  formatted = boost::python::str("\n").join(formatted_list);
  return boost::python::extract<std::string>(formatted);
}

namespace qi {
  namespace py {

    //Allow calling python from an unknown thread
    // (acquire the GIL and setup the thread)
    class GILScopedLock
    {
    public:
      GILScopedLock() {
        qiLogCategory("qi.py.gil");
        qiLogDebug() << "ScopedLockEnter(Begin)";
        _state = PyGILState_Ensure();
        qiLogDebug() << "ScopedLockEnter(End)";
      }

      ~GILScopedLock() {
        qiLogCategory("qi.py.gil");
        qiLogDebug() << "ScopedLockQuit(Begin)";
        PyGILState_Release(_state);
        qiLogDebug() << "ScopedLockQuit(End)";
      }

    private:
      PyGILState_STATE _state;
    };

    //Unlock the GIL, allow python to process other thread
    //SHOULD NEVER BE CALLED TWICE, DO NOT NEST
    class GILScopedUnlock
    {
    public:
      GILScopedUnlock()
        : _save(0)
      {
        qiLogCategory("qi.py.gil");
        qiLogDebug() << "ScopedUnLockEnter(Begin)";
        _save = PyEval_SaveThread();
        qiLogDebug() << "ScopedUnLockEnter(End)";
      }

      ~GILScopedUnlock() {
        qiLogCategory("qi.py.gil");
        qiLogDebug() << "ScopedUnLockQuit(Begin)";
        PyEval_RestoreThread(_save);
        qiLogDebug() << "ScopedUnLockQuit(End)";
      }

    private:
      PyThreadState* _save;
    };
  }
}


#endif	    /* !GIL_HPP_ */
