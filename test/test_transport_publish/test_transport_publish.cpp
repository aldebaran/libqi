/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <qi/transport/zeromq/zmqpublisher.hpp>
#include <qi/transport/zeromq/zmqsubscriber.hpp>
#include <qi/perf/sleep.hpp>
#include <boost/timer.hpp>

struct SubscribePerfHandler : qi::transport::ISubscribeHandler {
  int fCount;
  int fExpectedMessages;
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

  int getCount() {
    return fCount;
  }

  SubscribePerfHandler() : fCount(0) {}

  SubscribePerfHandler(int expectedMessages) :
    fCount(0),
    fExpectedMessages(expectedMessages) {}
};

TEST(TransportZMQPublisher , MillionPerSecond)
{
  int numMillions = 1;
  int numMessages = numMillions * 1000000;

  SubscribePerfHandler handler(numMessages);
  qi::transport::ZMQPublisher publisher1("tcp://127.0.0.1:5556");
  qi::transport::ZMQPublisher publisher("tcp://127.0.0.1:5555");
  qi::transport::ZMQSubscriber subscriber("tcp://127.0.0.1:5555");

  subscriber.setSubscribeHandler(&handler);
  subscriber.subscribe();
  std::string msg = "Hello";
  sleep(1);
  for (int i=0; i < numMessages; i++) {
    publisher.publish(msg);
  }

  sleep(2);
  int result = handler.getCount();
  ASSERT_EQ( numMessages, result) << "Did not receive all messages";
}



TEST(TransportZMQPublisher , MultipleSubscribers)
{
  int numMessages = 100000;

  qi::transport::ZMQPublisher publisher("tcp://127.0.0.1:5555");

  const int numSubscribers = 50;
  std::vector<SubscribePerfHandler*>         handlers;
  std::vector< qi::transport::ZMQSubscriber*> subscribers;
  boost::shared_ptr<zmq::context_t> subContext(new zmq::context_t(1));
  for (unsigned int i = 0; i < numSubscribers; ++i) {
    SubscribePerfHandler* hand = new SubscribePerfHandler(numMessages);
    qi::transport::ZMQSubscriber* sub = new qi::transport::ZMQSubscriber(subContext, "tcp://127.0.0.1:5555");
    sub->setSubscribeHandler(hand);
    sub->subscribe();
    handlers.push_back(hand);
    subscribers.push_back(sub);
  }
  sleep(1);
  std::string msg = "Hello";

  std::cout << "Publishing...";
  for (int i=0; i < numMessages; i++) {
    publisher.publish(msg);
  }
  std::cout << " Done." << std::endl;
  sleep(1);
  int result = 0;
  for(unsigned int i=0; i < numSubscribers; ++i) {
    result += handlers[i]->getCount();
  }
  ASSERT_EQ( numMessages * numSubscribers, result) << "Did not receive all messages";
}

