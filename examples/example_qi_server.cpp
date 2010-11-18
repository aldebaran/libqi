/** This is a simple Qi messaging example
 * It create a server and bind a foo method
 */

#include <qi/nodes/server_node.hpp>

/// simple method that will be binded
int foo(const int &bar)
{
  return bar + 1;
}

int main(int argc, char *argv[])
{
  qi::ServerNode server("myserver", "127.0.0.1:5595", "127.0.0.1:5509");

  server.addService("mymethod", &foo);
  sleep(5);
  return 0;
}
