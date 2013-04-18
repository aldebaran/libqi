/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/


#ifndef   	GIL_HPP_
# define   	GIL_HPP_

#include <boost/python.hpp>

namespace qi {
  namespace py {
    class GILScopedLock
    {
    public:
      GILScopedLock() : fState(PyGILState_Ensure()){}
      ~GILScopedLock() {PyGILState_Release(fState);}
    private:
      PyGILState_STATE fState;
    };

    class GILScopedUnlock
    {
    public:
      GILScopedUnlock() : fSave(PyEval_SaveThread()) {}
      ~GILScopedUnlock() {PyEval_RestoreThread(fSave);}
    private:
      PyThreadState* fSave;
    };
  }
}


#endif	    /* !GIL_HPP_ */
