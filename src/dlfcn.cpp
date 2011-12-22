/*
 * Copyright (c) 2011, Aldebaran Robotics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Aldebaran Robotics nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
      boost::filesystem::path fname(libNameToFileName(filename), qi::unicodeFacet());

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
