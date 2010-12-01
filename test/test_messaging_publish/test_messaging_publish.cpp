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
std::string gServerName = "server";
std::string gServerAddress = "127.0.0.1:5556";

Master gMaster(gMasterAddress);
Server gServer(gServerName, gMasterAddress);
Client gClient("client", gMasterAddress);


void handler(const std::string& msg) {
  std::cout << "Received : " << msg << std::endl;
}

TEST(MessagingPublisher , publisher)
{
  Subscriber<std::string> suscriber = gClient.subscribe<std::string>("hello", &handler);
  Publisher<std::string> publisher = gServer.advertiseTopic<std::string>("hello");
  //suscriber.subscribeHandler(std::string("a message"));

  sleep(1);
  for(unsigned int i=0; i<20; i++) {
    publisher.publish("hello hello");
    sleep(0.5f);
  }

  
}