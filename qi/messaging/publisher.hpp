/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_MESSAGING_PUBLISHER_HPP__
#define   __QI_MESSAGING_PUBLISHER_HPP__

#include <string>
#include <boost/shared_ptr.hpp>
#include <qi/serialization.hpp>

namespace qi {
  namespace detail {
    class PublisherImpl;
  }

  class Publisher {
  public:
    Publisher(const std::string& name, const std::string& masterAddress = "127.0.0.1:5555");
    virtual ~Publisher();

    template<typename T>
    void advertise(const std::string& topicName)
    {
      void (*f)(const T &p0)  = 0;
      std::string signature = makeSignature(topicName, f);
      xAdvertise(signature);
    }

    template<typename T>
    void publish(const std::string topicName, const T& val)
    {
      //typedef void(C::*FunctionType) (const T &p0);
      void (*f)(const T &p0)  = 0;
      qi::serialization::BinarySerializer ser;
      qi::serialization::serialize<std::string>::write(ser, makeSignature(topicName, f));
      qi::serialization::serialize<T>::write(ser, val);
      xPublish(ser.str());
    }

  protected:
    void xAdvertise(const std::string& signature);
    void xPublish(const std::string& message);
    boost::shared_ptr<qi::detail::PublisherImpl> _impl;
  };
}

#endif // __QI_MESSAGING_PUBLISHER_HPP__
