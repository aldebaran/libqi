#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_TRANSPORT_BUFFER_HPP_
#define _QI_TRANSPORT_BUFFER_HPP_

#include <string>

namespace qi {
  namespace transport {
    // use a string for the moment, but this will be a zerocopybuffer.
    typedef std::string Buffer;
  }
}

#endif  // _QI_TRANSPORT_BUFFER_HPP_
