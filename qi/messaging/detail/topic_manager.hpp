#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_TOPIC_MANAGER_HPP__
#define   __QI_MESSAGING_DETAIL_TOPIC_MANAGER_HPP__

#include <string>
#include <qi/messaging/detail/mutexednamelookup.hpp>
#include <qi/transport/detail/network/endpoint_context.hpp>
#include <qi/messaging/detail/topic_info.hpp>

namespace qi {
  namespace detail {

    class TopicManager {
    public:
      TopicManager();
      virtual ~TopicManager();

      const TopicInfo& getTopicInfo(const std::string& topicName);
      //void registerSubscriber(const std::string& topicName);
      //void registerPublisher(const std::string& topicName);

    private:
      MutexedNameLookup<TopicInfo> _topics;
    };

  }
}

#endif // __QI_MESSAGING_DETAIL_TOPIC_MANAGER_HPP__
