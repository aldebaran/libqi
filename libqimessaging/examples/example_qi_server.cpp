#include <qi/messaging/server.hpp>

// The handler for the service that you want to advertise
int getMeaningOfLife() {
  return 42;
}

int main(int argc, char *argv[])
{
  // Create the server, giving it a name that helps to track it
  qi::Server server("deepThought");

  // Connect the client to the master
  const std::string masterAddress = "127.0.0.1:5555";
  server.connect(masterAddress);

  // Advertise the service, giving it a name.
  server.advertiseService("deepThought.getMeaningOfLife", &getMeaningOfLife);
  // FIXME: need a nice way to sleep
}
