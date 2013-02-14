/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/os.hpp>
#include <Windows.h>

namespace qi {
  namespace os {

    int checkdbg() {
      return IsDebuggerPresent();
      //TODO: this function is not implemented under windows
      return 0;
    }

  }
};


