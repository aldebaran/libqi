/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <vector>
#include <iostream>
#include <cmath>
#include <qi/transport/zeromq/zmqpublisher.hpp>
#include <qi/perf/sleep.hpp>

int main(int argc, char **argv)
{
  qi::transport::ZMQPublisher publisher("tcp://127.0.0.1:5555");
  for (int i=0; i < 20; i++) {
    publisher.publish(std::string("Hello"));
  }
  return 0;
}
