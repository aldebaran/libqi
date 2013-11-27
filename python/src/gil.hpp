#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _PYTHON_SRC_GIL_HPP_
#define _PYTHON_SRC_GIL_HPP_

#include <qi/log.hpp>
#include <boost/python.hpp>

namespace qi {
  namespace py {

    //Acquire the GIL if needed
    //and setup the thread in the python world if needed
    //
    //Use that before calling python (even from an unknown thread)
    //Can be nested
    class GILScopedLock : private boost::noncopyable
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

    //Unlock the GIL, allow python to process other python threads
    //Use that while doing C++ computation (sleep, IO, whatever).
    //
    //This can be called from a thread not previously managed by python.
    //Can be nested
    class GILScopedUnlock : private boost::noncopyable
    {
    public:
      GILScopedUnlock();
      ~GILScopedUnlock();

    private:
      PyThreadState* _save;
    };
  }
}


#endif  // _PYTHON_SRC_GIL_HPP_
