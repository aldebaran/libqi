/**
** Copyright (C) 2010 Aldebaran Robotics
*/
/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <qi/messaging.hpp>
#include <qi/perf/dataperftimer.hpp>
#include <qi/perf/sleep.hpp>
#include <string>

using namespace qi;

Master gMaster;


void printHandler(const std::string& msg) {
  std::cout << "printHandler: " << msg << std::endl;
}

void handler1(const std::string& msg) {
//  std::cout << msg << "  Subscriber 1H" << std::endl;
}

void handler2(const std::string& msg) {
//  std::cout << msg << "  Subscriber 1G" << std::endl;
}

void handler3(const std::string& msg) {
//  std::cout << msg << "  Subscriber 2G" << std::endl;
}

TEST(MessagingPublisher, simpleCase) {
  gMaster.run();
  Subscriber sub("subscriber");
  Publisher pub("publisher");
  pub.connect();
  sub.connect();
  pub.advertiseTopic<std::string>("hello");
  sub.subscribe("hello", &printHandler);
  sleep(1.0); // FIXME
  pub.publish("hello", std::string("world"));

}

TEST(MessagingPublisher , twoSubscribers)
{
  Subscriber s("subscriber1");
  Subscriber s2("subscriber2");
  Publisher p("publisher");
  p.connect();
  s.connect();
  s2.connect();

  p.advertiseTopic<std::string>("hello");
  p.advertiseTopic<std::string>("goodbye");
  s.subscribe("hello", &handler1);
  s.subscribe("goodbye", &handler2);
  s2.subscribe("goodbye", &handler3);
  sleep(2.0);

  // TODO implement topic subscriptions in zmq subscriber,
  // so that you don't receive all messages from a publisher
  // when you are only subscribed to one of its messages
  for (int i = 0 ; i < 10 ; i++) {
    std::stringstream str;
    str << i;
    p.publish("hello", str.str());
    p.publish("goodbye", str.str());
  }
  sleep(2.0);
}