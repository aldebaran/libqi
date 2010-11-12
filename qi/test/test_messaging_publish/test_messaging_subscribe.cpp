/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <qi/messaging/generic_subscriber.hpp>
#include <qi/perf/sleep.hpp>


void subscribeHandler(const int& msg) {
  std::cout << "Handler received: " << msg << std::endl;
}

int main(int argc, char **argv)
{
  qi::messaging::GenericSubscriber<int> subscriber;
  subscriber.connect("tcp://127.0.0.1:5555");
  subscriber.subscribe(&subscribeHandler);
  while(true) {
    sleep(1);
  }
  return 0;
}
