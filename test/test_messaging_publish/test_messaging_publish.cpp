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


TEST(MessagingPublisher, initMaster) {
  gMaster.run();
}

TEST(MessagingPublisher, simpleCase) {
  Publisher pub("publisher");
  Subscriber sub("subscriber");

  pub.connect();
  sub.connect();
  pub.advertiseTopic<std::string>("hello");
  sub.subscribe("hello", &printHandler);
  sleep(1.0); // FIXME
  pub.publish("hello", std::string("world"));
  sleep(1.0);
}

TEST(MessagingPublisher , twoSubscribers)
{
  Publisher p("publisher");
  Subscriber s1("subscriber1");
  Subscriber s2("subscriber2");

  p.connect();
  s1.connect();
  s2.connect();

  p.advertiseTopic<std::string>("hello");
  p.advertiseTopic<std::string>("goodbye");
  
  //sleep(3);
  s1.subscribe("hello", &printHandler);
  s1.subscribe("goodbye", &handler2);
  s2.subscribe("goodbye", &handler3);
  sleep(2.0);

  //// TODO implement topic subscriptions in zmq subscriber,
  //// so that you don't receive all messages from a publisher
  //// when you are only subscribed to one of its messages
  for (int i = 0 ; i < 10 ; i++) {
    std::stringstream str;
    str << i;
    p.publish("hello", str.str());
    p.publish("goodbye", str.str());
  }
  std::cout << "All published" << std::endl;
  sleep(2.0);
}


TEST(MessagingPublisher , manyToMany)
{
  Subscriber s1("subscriber1");
  Subscriber s2("subscriber2");
  Publisher p1("publisher1");
  Publisher p2("publisher2");
  p1.connect();
  p2.connect();
  s1.connect();
  s2.connect();

  p1.advertiseTopic<std::string>("chat", true);
  p2.advertiseTopic<std::string>("chat", true);

  s1.subscribe("chat", &printHandler);
  s2.subscribe("chat", &printHandler);

  for (int i = 0 ; i < 10 ; i++) {
    std::stringstream sstream1;
    sstream1 << "Chat from publisher 1 " << i;
    p1.publish("chat", sstream1.str());

    std::stringstream sstream2;
    sstream2 << "Chat from publisher 2 " << i;
    p2.publish("chat", sstream2.str());
  }
  std::cout << "All published" << std::endl;
  sleep(2.0);
  std::cout << "Exiting" << std::endl;
}