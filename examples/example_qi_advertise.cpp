#include <qi/messaging/server.hpp>

// The handler that you want to bind
int getMeaningOfLife() {
  return 42;
}

int main(int argc, char *argv[])
{
  // Create the server, giving it a name that helps to track it
  qi::Server server("deepThought");
  // Add the service, giving it a name.
  server.addService("deepThought.getMeaningOfLife", &getMeaningOfLife);
  // FIXME: need a nice way to sleep
}
