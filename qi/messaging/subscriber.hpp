/*
** Author(s):
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_MESSAGING_SUBSCRIBER_HPP__
#define   __QI_MESSAGING_SUBSCRIBER_HPP__

#include <string>
#include <boost/shared_ptr.hpp>
#include <qi/transport/subscribe_handler.hpp>
#include <qi/transport/subscribe_handler_user.hpp>
#include <qi/serialization/serializer.hpp>
#include <boost/function.hpp>
#include <qi/log.hpp>

namespace qi {

    template<typename SUBSCRIBE_TYPE>
    class Subscriber : public qi::transport::SubscribeHandler {
    public:
      Subscriber(boost::shared_ptr<qi::transport::SubscribeHandlerUser> impl) : _impl(impl) {}
      Subscriber(const Subscriber& rhs) : _impl(rhs._impl), _callback(rhs._callback) {
        std::cout << "bla";
      }
      virtual ~Subscriber() {}
      void subscribeHandler(const std::string& data) {
        SUBSCRIBE_TYPE ret;
        qi::serialization::BinarySerializer unser(data);
        qi::serialization::serialize<SUBSCRIBE_TYPE>::read(unser, ret);
        _callback(ret);
      }

      void setCallback(boost::function<void (const SUBSCRIBE_TYPE&)> f) {
        _callback = f;
      }

    protected:
      boost::function<void (const SUBSCRIBE_TYPE&)> _callback;
      boost::shared_ptr<qi::transport::SubscribeHandlerUser> _impl;
    };
}

#endif // __QI_MESSAGING_SUBSCRIBER_HPP__
