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
#include "src/filesystem.hpp"

namespace qi {
  namespace os {

    static std::string libNameToFileName(const std::string &refLibraryName)
    {
     #ifdef _WIN32
      static const char szPre[] = "";
      static const char szExt[] = ".dll";
     #endif
     #ifdef __APPLE__
      static const char szPre[] = "lib";
      static const char szExt[] = ".dylib";
     #endif
     #ifdef __linux__
      static const char szPre[] = "lib";
      static const char szExt[] = ".so";
     #endif
      if (refLibraryName.find(szExt) != std::string::npos)
        return refLibraryName;
      return szPre + refLibraryName + szExt;
    }


    void *dlopen(const char *filename, int flag) {
      void *handle = NULL;
      boost::filesystem::path fname(filename, qi::unicodeFacet());
      std::string file = libNameToFileName(fname.filename().string());
      fname = fname.parent_path() / file;
      qiLogDebug("qi.dlopen") << "opening " << fname;
     #ifdef WIN32
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
     #ifdef WIN32
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
     #ifdef WIN32
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
