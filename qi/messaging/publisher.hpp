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
  class Context;

  /// <summary>Used to distribute data on named Topics.
  /// Advertised Topics are registered with the master so that
  /// subscribers can find them.</summary>
  /// \ingroup Messaging
  class Publisher {
  public:

    /// <summary> Create a Publisher that can be used to distribute data
    /// on named Topics
    /// </summary>
    /// <param name="name"> The name of the publisher </param>
    /// <param name="context">
    /// An optional context that can be used to group or separate
    /// transport resources.
    /// </param>
    Publisher(const std::string &name = "", qi::Context *context = 0);
    virtual ~Publisher();

    /// <summary> Reset to the default state, this will disconnect
    /// and reset the object, like a new fresh copy. </summary>
    /// <param name="name"> The name of the publisher </param>
    /// <param name="context">
    /// An optional context that can be used to group or separate
    /// transport resources.
    /// </param>
    void reset(const std::string &name = "", qi::Context *context = 0);

    /// <summary> Connect to masterAddress. If no address is specified
    /// the default 127.0.0.1:5555 is used </summary>
    /// <param name="masterAddress"> The master address. e.g 127.0.0.1:5555
    /// </param>
    void connect(const std::string &masterAddress = "127.0.0.1:5555");

    /// <summary>Advertises to the master that you wish to publish data
    /// of type "PUBLISH_TYPE" to a topic of name "topicName".</summary>
    /// <param name="topicName">The name of the topic you wish to publish to.</param>
    /// <seealso cref="qi::Subscriber"/>
    /// <seealso cref="qi::Master"/>
    template<typename PUBLISH_TYPE>
    void advertiseTopic(const std::string& topicName)
    {
      void (*f)(const PUBLISH_TYPE &p0)  = 0;
      std::string signature = makeSignature(topicName, f);
      xAdvertiseTopic(signature);
    }

    /// <summary> Unadvertises a topic</summary>
    /// <param name="topicName">The name of the topic you wish to
    /// unadvertise.</param>
    template<typename PUBLISH_TYPE>
    void unadvertiseTopic(const std::string& topicName)
    {
      void (*f)(const PUBLISH_TYPE &p0)  = 0;
      std::string signature = makeSignature(topicName, f);
      xUnadvertiseTopic(signature);
    }

    /// <summary>
    /// Publishes messages to an existing topic.
    ///
    /// e.g. publisher.publish("/time/hour_of_the_day", 10);
    ///
    /// A subscriber would be able to subscribe to the above with
    ///
    /// subscriber.subscribe("time/hour_of_the_day", &handler);
    ///
    /// where the handler expects an int type.
    /// </summary>
    /// <param name="topicName">The name of the topic you want to publish to.
    ///
    /// By convention, topic names use forward slashes as a separator
    /// e.g. "/time/hour_of_the_day"
    /// </param>
    /// <param name="publishData">The typed data that you wish to publish</param>
    /// <returns>void</returns>
    /// <seealso cref="qi::Subscriber"/>
    template<typename PUBLISH_TYPE>
    void publish(const std::string topicName, const PUBLISH_TYPE& publishData)
    {
      void (*f)(const PUBLISH_TYPE &p0)  = 0;
      qi::serialization::Message ser;
      qi::serialization::serialize<std::string>::write(ser, makeSignature(topicName, f));
      qi::serialization::serialize<PUBLISH_TYPE>::write(ser, publishData);
      xPublish(ser.str());
    }

  protected:
    /// <summary>Advertises a Topic </summary>
    /// <param name="signature">The signature of the topic</param>
    void xAdvertiseTopic(const std::string& signature);

    /// <summary>Unadvertises a Topic. </summary>
    /// <param name="signature">The signature of the topic.</param>
    void xUnadvertiseTopic(const std::string& signature);

    /// <summary> Publishes a serialized message </summary>
    /// <param name="message">The message.</param>
    void xPublish(const std::string& message);

    /// <summary> The private implementation </summary>
    boost::shared_ptr<qi::detail::PublisherImpl> _impl;
  };
}

#endif  // _QI_MESSAGING_PUBLISHER_HPP_
