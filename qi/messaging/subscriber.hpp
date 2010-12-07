#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SUBSCRIBER_HPP_
#define _QI_MESSAGING_SUBSCRIBER_HPP_

#include <string>
#include <boost/scoped_ptr.hpp>
#include <qi/signature.hpp>
#include <qi/functors/makefunctor.hpp>

namespace qi {
  namespace detail {
    class SubscriberImpl;
  }

  /// <summary>
  /// Used to subscribe to services.
  /// </summary>
  class Subscriber {
  public:
    /// <summary>
    /// DefaultConstructor
    /// </summary>
    Subscriber();

    /// <summary>
    /// TODO
    /// </summary>
    /// <param name="subscriberName">
    /// The name you want to give to this subscriber
    /// e.g. "subscriber"
    /// </param>
    /// <param name="masterAddress">
    /// The address of the master that is used to find publishers
    /// e.g. "127.0.0.1:5555"
    /// </param>
    Subscriber(const std::string& subscriberName, const std::string& masterAddress = "127.0.0.1:5555");

    virtual ~Subscriber();

    bool isInitialized() const;

    template<typename FUNCTION_TYPE>
    void subscribe(const std::string& topicName, FUNCTION_TYPE callback)
    {
      xSubscribe(makeSignature(topicName, callback), makeFunctor(callback));
    }

    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    void subscribe(const std::string& topicName, OBJECT_TYPE object, METHOD_TYPE callback)
    {
      xSubscribe(makeSignature(topicName, callback), makeFunctor(object, callback));
    }

  private:
    void xSubscribe(const std::string& topicName, Functor* f);
    boost::scoped_ptr<qi::detail::SubscriberImpl> _impl;
  };
}

#endif  // _QI_MESSAGING_SUBSCRIBER_HPP_
