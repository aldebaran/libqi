#include <iostream>
#include <qi/messaging.hpp>

static const std::string masterAddress = "127.0.0.1:5555";

// The handler for the service that you want to advertise
int getMeaningOfLife() {
  return 42;
}

int main(int argc, char *argv[])
{
  qi::Master master;
  qi::Server server("myServer");
  qi::Client client("myClient");

  master.run();

  server.connect(masterAddress);
  server.advertiseService("deepThought.getMeaningOfLife", &getMeaningOfLife);
  client.connect(masterAddress);

  int theMeaningOfLife = client.call<int>("deepThought.getMeaningOfLife");
  return theMeaningOfLife;
}
