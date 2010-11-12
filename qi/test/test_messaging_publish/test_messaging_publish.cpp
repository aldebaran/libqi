/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <vector>
#include <iostream>
#include <cmath>
#include <qi/messaging/generic_publisher.hpp>
#include <qi/perf/sleep.hpp>

int main(int argc, char **argv)
{
  qi::messaging::GenericPublisher<int> publisher;
  publisher.connect("tcp://127.0.0.1:5555");
  for (int i=0; i < 20; i++) {
    publisher.publish(i);
  }
  return 0;
}
