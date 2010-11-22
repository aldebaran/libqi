/** This is a simple Qi messaging example
 * It creates a server and binds a foo method
 */

#include <qi/messaging/server.hpp>
#include <qi/perf/sleep.hpp>

/// simple method that will be bound
int foo(const int &bar)
{
  return bar + 1;
}

int main(int argc, char *argv[])
{
  qi::Server server("myserver", "127.0.0.1:5509");

  server.addService("mymethod", &foo);
  sleep(5);
  return 0;
}
