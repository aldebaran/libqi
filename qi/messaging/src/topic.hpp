#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SRC_TOPIC_HPP_
#define _QI_MESSAGING_SRC_TOPIC_HPP_

#include <vector>
#include <string>

namespace qi {
  namespace detail {
    struct Topic {
      std::string topicName;
      std::string publishEndpointID;
      std::string subscribeEndpointID;
      std::vector<std::string> publisherIDs;
      std::vector<std::string> subscriberIDs;
    };
  }
}
#endif  // _QI_MESSAGING_SRC_TOPIC_HPP_

