/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <qi/os.hpp>
#include <qi/types.hpp>

namespace qi {
  namespace os {
    /* Have the static variable we need inside the function so that we
     * give correct results at initialization time, but also force
     * initialization so that timestamp 0 corresponds to pogram start time.
     */
    static int64_t _unused_base = ustime();

    int64_t ustime()
    {
      static bool initialized = false;
      static int64_t base;
      timeval tv;
      gettimeofday(&tv);
      if (!initialized)
      {
        base = tv.tv_sec*1000000LL + tv.tv_usec;
        initialized = true;
      }
      return tv.tv_sec*1000000LL + tv.tv_usec - base;
    }
  }
}
