#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_TOPIC_INFO_HPP__
#define   __QI_MESSAGING_DETAIL_TOPIC_INFO_HPP__

#include <string>
#include <vector>

namespace qi {
  namespace detail {
    struct TopicInfo {
      std::string topicName;
      std::string publishEndpointID;
      std::string subscribeEndpointID;
      std::vector<std::string> publisherIDs;
      std::vector<std::string> subscriberIDs;
      bool isManyToMany;
      TopicInfo() : isManyToMany(false) {}
    };
  }
}

#endif // __QI_MESSAGING_DETAIL_TOPIC_INFO_HPP__
