/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Cedric GESTES
*/

#include <qi/os.hpp>
#include <Windows.h>

namespace qi {
  namespace os {

    int check_debugger() {
	  return IsDebuggerPresent();
      //TODO: this function is not implemented under windows
      return 0;
    }

  }
};


