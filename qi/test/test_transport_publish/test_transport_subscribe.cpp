/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <qi/transport/zeromq/zmqsubscriber.hpp>

struct Handler : qi::transport::ISubscribeHandler {
  void subscribeHandler(const std::string & msg) {
    std::cout << "Handler received: " << msg << std::endl;
  }
};

Handler h;

int main(int argc, char **argv)
{
  qi::transport::ZMQSubscriber subscriber("tcp://127.0.0.1:5555");
  subscriber.setSubscribeHandler(&h);
  subscriber.subscribe();
  return 0;
}
