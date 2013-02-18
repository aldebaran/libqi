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
        return 0;
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
      return FreeLibrary((HINSTANCE) handle);
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
      DWORD result = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, lastError, 0, err, sizeof(err), 0);
      return err;
     #else
      return ::dlerror();
     #endif
    }

  }
}
