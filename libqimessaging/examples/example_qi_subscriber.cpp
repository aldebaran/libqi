#include <qimessaging/subscriber.hpp>

// The handler for the topic data
void hourCallback(const int& hour) {
  // do something with the hour
}

int main()
{
  // Create the subscriber, giving it a name that helps to track it
  qi::Subscriber subscriber("hourSubscriber");

  // Connect the subscriber to the master
  const std::string masterAddress = "127.0.0.1:5555";
  subscriber.connect(masterAddress);

  // Subscribe to the topic, providing the callback handler
  // The handler's argument type of "int" is used to give a strong type
  // to your topic subscription
  subscriber.subscribe("time/hour", &hourCallback);
  // Whenever a publisher publishes an int to "time/hour" your callback
  // will be called.

  // FIXME: need a nice way to sleep
}
