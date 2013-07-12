/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/log.hpp>
#include <qi/os.hpp>

#include <boost/filesystem.hpp>

#include <iostream>
#include <cstring>

# ifdef _WIN32
#  include <windows.h>
# else
#  include <dlfcn.h>
# endif

#include <qi/qi.hpp>
#include <qi/path.hpp>

#include "filesystem.hpp"

namespace qi {
  namespace os {

    void *dlopen(const char *filename, int flag) {
      std::string fullName = path::findLib(filename);
      if (fullName.empty())
      {
        qiLogError("qi.dlopen") << "Could not locate library " << filename;
        fullName = filename; // Do not return here, let sys call fails and set errno.
      }
      void *handle = NULL;
      boost::filesystem::path fname(fullName, qi::unicodeFacet());
      qiLogDebug("qi.dlopen") << "opening " << fname;
     #ifdef _WIN32
      handle = LoadLibraryW(fname.wstring(qi::unicodeFacet()).c_str());
     #else
      if (flag == -1)
        flag = RTLD_NOW;
      handle = ::dlopen(fname.string(qi::unicodeFacet()).c_str(), flag);
     #endif
      return handle;
    }

    int   dlclose(void *handle) {
      if (!handle)
        return 0;
     #ifdef _WIN32
      // Mimic unix dlclose (0 on success)
      return FreeLibrary((HINSTANCE) handle) != 0 ? 0 : -1;
     #else
      return ::dlclose(handle);
     #endif
    }

    void *dlsym(void *handle, const char *symbol) {
      void* function = NULL;

      if(!handle)
        return 0;
     #ifdef _WIN32
      function = (void *)GetProcAddress((HINSTANCE) handle, symbol);
     #else
      function = ::dlsym(handle, symbol);
     #endif
      return function;
    }

    const char *dlerror(void) {
     #ifdef _WIN32
      static char err[255];
      DWORD lastError = GetLastError();
      // Unix dlerror() return null if error code is 0
      if (lastError == 0)
        return NULL;

      DWORD result = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, lastError, 0, err, sizeof(err), 0);
      // Unix dlerror() resets its value after a call, ensure same behavior
      SetLastError(0);
      return err;
     #else
      return ::dlerror();
     #endif
    }

  }
}
