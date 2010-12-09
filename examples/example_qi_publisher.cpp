#include <qi/messaging/publisher.hpp>

int main(int argc, char *argv[])
{
  // Create the publisher, giving it a name that helps to track it
  qi::Publisher publisher("time");

  // Connect the publisher to the master
  const std::string masterAddress = "127.0.0.1:5555";
  publisher.connect(masterAddress);

  // Advertise the Topic, giving it a name and an int type
  publisher.advertiseTopic<int>("time/hour");

  // Publish the hour to the Topic, sending an int
  publisher.publish("time/hour", 10);
}
