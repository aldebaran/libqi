#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_PUBLISHER_HPP_
#define _QI_MESSAGING_PUBLISHER_HPP_

#include <string>
#include <boost/shared_ptr.hpp>
#include <qi/serialization.hpp>

namespace qi {
  namespace detail {
    class PublisherImpl;
  }

  class Publisher {
  public:
    Publisher();
    Publisher(const std::string& name, const std::string& masterAddress = "127.0.0.1:5555");
    virtual ~Publisher();

    /// <summary>Advertises to the master that you wish to publish data
    /// of type "PUBLISH_TYPE" to a topic of name "topicName".</summary>
    /// <param name="topicName">The name of the topic you wish to publish to.</param>
    template<typename PUBLISH_TYPE>
    void advertiseTopic(const std::string& topicName)
    {
      void (*f)(const PUBLISH_TYPE &p0)  = 0;
      std::string signature = makeSignature(topicName, f);
      xAdvertiseTopic(signature);
    }

    /// <summary>
    /// Publishes messages to an existing topic.
    /// 
    /// e.g. publisher.publish("/time/hour_of_the_day", 10);
    /// 
    /// A subscriber would be able to subscribe to the above with
    /// subscriber.subscribe("time/hout_of_the_day", &handler);
    /// where the handler expects an int type.
    /// </summary>
    /// <param name="topicName">The name of the topic you want to publish to.
    /// 
    /// By convention, topic names use forward slashes as a separator
    /// e.g. "/time/hour_of_the_day"
    /// </param>
    /// <param name="publishData">The typed data that you wish to publish</param>
    /// <returns>void</returns>
    template<typename PUBLISH_TYPE>
    void publish(const std::string topicName, const T& publishData)
    {
      void (*f)(const PUBLISH_TYPE &p0)  = 0;
      qi::serialization::Message ser;
      qi::serialization::serialize<std::string>::write(ser, makeSignature(topicName, f));
      qi::serialization::serialize<PUBLISH_TYPE>::write(ser, publishData);
      xPublish(ser.str());
    }

  protected:
    void xAdvertiseTopic(const std::string& signature);
    void xPublish(const std::string& message);
    boost::shared_ptr<qi::detail::PublisherImpl> _impl;
  };
}

#endif  // _QI_MESSAGING_PUBLISHER_HPP_
