#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_NETWORK_PLATFORM_HPP__
#define   __QI_MESSAGING_DETAIL_NETWORK_PLATFORM_HPP__

namespace qi {
  namespace detail {

    enum Platform {
      PlatformWindows = 0,
      platformMac     = 1,
      PlatformLinux   = 2,
      PlatformUnknown = 99
    };

    /// <summary> Gets the platform </summary>
    inline int getPlatform() {
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
#endif // __QI_MESSAGING_DETAIL_NETWORK_PLATFORM_HPP__

