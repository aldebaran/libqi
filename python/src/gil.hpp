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


#define PY_DISPLAY_ERROR(DO)                                \
try                                                         \
   {                                                        \
     DO;                                                    \
   }                                                        \
   catch (const boost::python::error_already_set &e)        \
   {                                                        \
     PyObject *ptype, *pvalue, *ptraceback;                 \
     PyErr_Fetch(&ptype, &pvalue, &ptraceback);             \
     qiLogError("python") << PyString_AsString(pvalue);     \
     PyErr_Restore(ptype, pvalue, ptraceback);              \
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
