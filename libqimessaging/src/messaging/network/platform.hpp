/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#pragma once
#ifndef _SRC_MESSAGING_NETWORK_PLATFORM_HPP_
#define _SRC_MESSAGING_NETWORK_PLATFORM_HPP_

namespace qi {
  namespace detail {

    enum Platform {
      PlatformWindows = 0,
      PlatformMac     = 1,
      PlatformLinux   = 2,
      PlatformUnknown = 99
    };

    inline std::string platformAsString(Platform p) {
      if (p == PlatformWindows) {
        return "Windows";
      }
      if (p == PlatformMac) {
        return "Mac";
      }
      if (p == PlatformMac) {
        return "Linux";
      }
      return "Unknown";
    }

    /// <summary> Gets the platform </summary>
    inline Platform getPlatform() {
#ifdef _WIN32
     return PlatformWindows;
#else
     // TODO MAC
     return PlatformLinux;
#endif
     return PlatformUnknown;
    }
  }
}
#endif  // _SRC_MESSAGING_NETWORK_PLATFORM_HPP_

