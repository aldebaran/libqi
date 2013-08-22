/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/
#include "gil.hpp"

#include <boost/thread/tss.hpp>

qiLogCategory("qi.py.gil");

namespace qi {
  namespace py {

    static boost::thread_specific_ptr<bool> gUnlocked;

    GILScopedUnlock::GILScopedUnlock()
      : _save(0)
    {
      //we want the use of GILScopedUnlock to be nested...
      //so we define a TLS bool that is set once we have locked once.

      //TLS not initialised for this thread? init...
      if (!gUnlocked.get()) {
        bool *b = new bool(false);
        gUnlocked.reset(b);
      }

      //GILScopedUnlock already called for this thread, continue.
      if (*gUnlocked == true)
        return;

      qiLogDebug() << "ScopedUnLockEnter(Begin)";
      //_save == 0 mean Python_FatalError
      _save = PyEval_SaveThread();

      if (_save) {
        //tell other call to GILSCopedUnlock that it's already called
        *gUnlocked = true;
      }
      //else python will notify us about the error

      qiLogDebug() << "ScopedUnLockEnter(End)";
    }

    GILScopedUnlock::~GILScopedUnlock() {
      qiLogDebug() << "ScopedUnLockQuit(Begin)";
      if (_save) {
        PyEval_RestoreThread(_save);
        *gUnlocked = false;
      }
      qiLogDebug() << "ScopedUnLockQuit(End)";
    }


  }
}
