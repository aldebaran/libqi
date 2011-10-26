/**
 * This is a simple QiMessaging example
 */

#include <qi/os.hpp>
#include <qimessaging/server.hpp>

// Handler for the service you want to advertise
int getMeaningOfLife()
{
  return 42;
}

int main()
{
  // Create the server, giving it a name that helps to track it
  qi::Server server("deepThought");

  // Connect the server to the master
  const std::string masterAddress = "127.0.0.1:5555";
  server.connect(masterAddress);

  // Advertise the service, giving it a name.
  server.advertiseService("deepThought.getMeaningOfLife", &getMeaningOfLife);

  // FIXME: need a nice way to sleep
  while (true)
    qi::os::sleep(1);
}
