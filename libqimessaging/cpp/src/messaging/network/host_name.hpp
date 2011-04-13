#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_NETWORK_HOST_NAME_HPP_
#define _QI_MESSAGING_SRC_NETWORK_HOST_NAME_HPP_

#include <string>

namespace qi {
  namespace detail {
    std::string getHostName();
  }
}
#endif  // _QI_MESSAGING_SRC_NETWORK_HOST_NAME_HPP_

