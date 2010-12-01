/**
** Copyright (C) 2010 Aldebaran Robotics
*/
/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include <boost/timer.hpp>
#include <qi/messaging/generic_publisher.hpp>
#include <qi/messaging/generic_subscriber.hpp>

#include <qi/perf/sleep.hpp>

  int fCount = 0;
  int fExpectedMessages = 50000;
  boost::timer timer;

// conforms to ISubscriberHandler
  void subscribeHandler(const std::string& msg) {
  if (fCount == 0) {
    timer.restart();
  }
  fCount++;
  if (fCount != fExpectedMessages) {
    return;
  }
  // print results
  double elapsed = timer.elapsed();
  double msgPerSecond = 1.0 /(elapsed / fExpectedMessages);
  std::cout << "SUB: msg/s: " <<
    std::setprecision(12) << msgPerSecond << std::endl;
}


TEST(MessagingZMQPublisher , perf)
{
  int numMessages = fExpectedMessages;

//  SubscribePerfHandler<int> handler(numMessages);
  qi::messaging::GenericPublisher<std::string> publisher;
  publisher.bind("tcp://127.0.0.1:5555");

  qi::messaging::GenericSubscriber<std::string> subscriber;
  subscriber.connect("tcp://127.0.0.1:5555");
  subscriber.subscribe(&subscribeHandler);
  std::string msg = "Hello";
  sleep(1);
  for (int i=0; i < numMessages; i++) {
    publisher.publish(msg);
  }

  sleep(1);
  ASSERT_EQ( numMessages, fCount) << "Did not receive all messages";
}
