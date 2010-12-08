/** This is a simple Qi messaging example
 * First we will create a qi Client, then use it to call a method.
 */

#include <qi/messaging/client.hpp>

int main(int argc, char *argv[])
{
  /// create a client
  qi::Client client("myclient");
  client.connect("127.0.0.1:44544");

  /// call mymethod
  int theMeaningOfLife = client.call<int>("deepThought.getMeaningOfLife");
  return 0;
}
