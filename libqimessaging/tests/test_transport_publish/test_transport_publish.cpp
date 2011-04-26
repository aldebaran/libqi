/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <gtest/gtest.h>
#include <vector>
#include <iostream>
#include <cmath>
#include "src/transport/zmq/zmq_publisher.hpp"
#include "src/transport/zmq/zmq_subscriber.hpp"
#include <qi/os.hpp>
#include <boost/timer.hpp>

struct SubscribePerfHandler : qi::transport::TransportSubscribeHandler {
  int fCount;
  int fExpectedMessages;
  boost::timer timer;

  // conforms to ISubscriberHandler
  void subscribeHandler(qi::transport::Buffer& msg) {
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
    std::cout << "SUB: msg:" << fCount << " msg/s: " <<
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

TEST(TransportZMQPublisher , HundredThousand)
{
  int numMillions = 1;
  int numMessages = numMillions * 100000;
  zmq::context_t ctx1(1);
  zmq::context_t ctx2(1);


  SubscribePerfHandler handler(numMessages);
  //qi::transport::ZMQPublisher publisher1("tcp://127.0.0.1:5556");
  qi::transport::detail::ZMQPublisher publisher(ctx1);
  publisher.bind("tcp://127.0.0.1:5555");
  qi::transport::detail::ZMQSubscriber subscriber(ctx2);
  subscriber.connect("tcp://127.0.0.1:5555");

  subscriber.setSubscribeHandler(&handler);
  subscriber.subscribe();
  std::string msg = "Hello";
  sleep(1);
  for (int i=0; i < numMessages; i++) {
    publisher.publish(msg);
  }

  sleep(10);
  int result = handler.getCount();
  ASSERT_EQ( numMessages, result) << "Did not receive all messages";
}



TEST(TransportZMQPublisher , MultipleSubscribers)
{
  int numMessages = 10000;

  const unsigned int numSubscribers = 10;
  std::cout << "Using " << numSubscribers << " subscribers" << std::endl;
  std::vector<SubscribePerfHandler*>         handlers;
  std::vector< qi::transport::detail::ZMQSubscriber*> subscribers;
  //boost::shared_ptr<zmq::context_t> subContext(new zmq::context_t(1));
  zmq::context_t ctx(1);
  for (unsigned int i = 0; i < numSubscribers; ++i) {
    SubscribePerfHandler* hand = new SubscribePerfHandler(numMessages);
    qi::transport::detail::ZMQSubscriber* sub = new qi::transport::detail::ZMQSubscriber(ctx);
    sub->connect("tcp://127.0.0.1:5555");
    sub->setSubscribeHandler(hand);
    sub->subscribe();
    handlers.push_back(hand);
    subscribers.push_back(sub);
  }
  qi::transport::detail::ZMQPublisher publisher(ctx);
  publisher.bind("tcp://127.0.0.1:5555");

  sleep(1);
  std::string msg = "Hello";

  std::cout << "Publishing...";
  for (int i=0; i < numMessages; i++) {
    publisher.publish(msg);
  }
  std::cout << " Done." << std::endl;
  sleep(10);
  int result = 0;
  for(unsigned int i=0; i < numSubscribers; ++i) {
    result += handlers[i]->getCount();
  }

  for(unsigned int i=0; i < numSubscribers; i++) {
    delete subscribers[i];
  }
  int expectedMessaged = numMessages * numSubscribers;
  ASSERT_EQ( expectedMessaged, result) << "Did not receive all messages";

}

