/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/log.hpp>
#include <qi/os.hpp>

#include <boost/filesystem.hpp>
#include <boost/thread/tss.hpp>

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

#ifdef _WIN32

    void cleanup(char* ptr)
    {
      delete[] ptr;
    }

#endif // !_WIN32

    const char *dlerror(void) {
     #ifdef _WIN32
      DWORD lastError = GetLastError();
      // Unix dlerror() return null if error code is 0
      if (lastError == 0)
        return NULL;

      static boost::thread_specific_ptr<char> err(cleanup);
      if (!err.get())
        err.reset(static_cast<char*>(new char[255]));

      DWORD result = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, lastError, 0, err.get(), sizeof(err.get()), 0);

      // Unix dlerror() resets its value after a call, ensure same behavior
      SetLastError(0);
      return err.get();
     #else
      return ::dlerror();
     #endif
    }

  }
}
