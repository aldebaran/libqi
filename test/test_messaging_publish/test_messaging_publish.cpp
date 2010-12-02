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

std::string gMasterAddress = "127.0.0.1:5555";
//std::string gServerName = "server";
//std::string gServerAddress = "127.0.0.1:5556";

Master gMaster(gMasterAddress);
//Server gServer(gServerName, gMasterAddress);
//Client gClient("client", gMasterAddress);


void handler1(const std::string& msg) {
  std::cout << msg << "  Subscriber 1H" << std::endl;
}

void handler2(const std::string& msg) {
  std::cout << msg << "  Subscriber 1G" << std::endl;
}

void handler3(const std::string& msg) {
  std::cout << msg << "  Subscriber 2G" << std::endl;
}

TEST(MessagingPublisher , publisher)
{
  Subscriber s("subscriber");
  Subscriber s2("subscriber");
  Publisher p("publisher");

  p.advertiseTopic<std::string>("hello");
  p.advertiseTopic<std::string>("goodbye");
  s.subscribe("hello", &handler1);
  s.subscribe("goodbye", &handler2);
  s2.subscribe("goodbye", &handler3);
  sleep(2.0);

  for (int i = 0 ; i < 50 ; i++) {
    std::stringstream s;
    s << i;
    p.publish("hello", s.str());
    p.publish("goodbye", s.str());
    sleep(0.1f);
  }

  sleep(2);
  //Publisher<std::string> publisher = gServer.advertiseTopic<std::string>("hello");
  //suscriber.subscribeHandler(std::string("a message"));

  //sleep(1);
  //for(unsigned int i=0; i<20; i++) {
  //  publisher.publish("hello hello");
  //  sleep(0.5f);
  //}

  
}