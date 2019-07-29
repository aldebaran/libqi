/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/log.hpp>
#include <qi/os.hpp>

#include <boost/predef/os.h>
#include <boost/filesystem.hpp>
#include <boost/thread/tss.hpp>

#include <cstring>

#if BOOST_OS_WINDOWS
#  include <windows.h>
#  include "os_win32.hpp"
#else
#  include <dlfcn.h>
#endif

#include <qi/path.hpp>


qiLogCategory("qi.dlfcn");

namespace qi {
  namespace os {

    void noop(char*) {}
    boost::thread_specific_ptr<char> g_LastError(&noop);

    void *dlopen(const char *filename, int flag) {
      g_LastError.reset();
      std::string fullName = path::findLib(filename);
      if (fullName.empty())
      {
        qiLogVerbose() << "Could not locate library " << filename;
        fullName = filename; // Do not return here, let sys call fails and set errno.
        if (fullName.empty())
        {
          // do not allow dlopen(""), it will return a valid handler to the
          // current process
          g_LastError.reset(const_cast<char*>("trying to dlopen empty filename"));
          return NULL;
        }
      }
      void *handle = NULL;
      boost::filesystem::path fname(fullName, qi::unicodeFacet());
      qiLogVerbose() << "opening " << fname;
#ifdef _WIN32
      handle = LoadLibraryW(fname.wstring().c_str());
#else
      if (flag == -1)
        flag = RTLD_NOW;
      handle = ::dlopen(fname.string().c_str(), flag);
#endif
      return handle;
    }

    int   dlclose(void *handle) {
      g_LastError.reset();
      if (!handle)
        return  0;
#ifdef _WIN32
      // Mimic unix dlclose (0 on success)
      return FreeLibrary((HINSTANCE) handle) != 0 ? 0 : -1;
#else
      return ::dlclose(handle);
#endif
    }

    void *dlsym(void *handle, const char *symbol) {
      g_LastError.reset();
      void* function = NULL;

      if(!handle)
      {
        g_LastError.reset(const_cast<char*>("null handle"));
        return  nullptr;
      }
#ifdef _WIN32
      function = (void *)GetProcAddress((HINSTANCE) handle, symbol);
#else
      function = ::dlsym(handle, symbol);
#endif
      return function;
    }

    const char *dlerror(void) {
#ifdef _WIN32
      /* GetLastError() must be called at the closest time after a system call it must retrieve error from
         From MSDN doc ( http://msdn.microsoft.com/en-us/library/windows/desktop/ms679360(v=vs.85).aspx ).

         g_LastError.get() modifies the result of GetLastError() so we make sure that
         GetLastError() is called first.
       */
      DWORD lastError = GetLastError();
#endif
      if (g_LastError.get())
      {
        const char* ret = g_LastError.get();
        g_LastError.reset();
        return ret;
      }
#ifdef _WIN32

      // Unix dlerror() return null if error code is 0
      if (lastError == 0)
        return NULL;

      // Keep alive the last message for this thread.
      static thread_local std::string errorMessage;
      errorMessage = translateSystemError(lastError);

      // Unix dlerror() resets its value after a call, ensure same behavior
      SetLastError(0);
      return errorMessage.c_str();
#else
      return ::dlerror();
#endif
    }

  }
}
