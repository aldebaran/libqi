/** This is a simple Qi messaging example
 * First we will create a qi Client, then use it to call a method.
 */

#include <qimessaging/client.hpp>

int main(int argc, char *argv[])
{
  // create a client
  qi::Client client("myClient");

  // Connect the client to the master
  const std::string masterAddress = "127.0.0.1:5555";
  client.connect(masterAddress);

  // call a method, expecting an int return type
  int theMeaningOfLife = client.call<int>("deepThought.getMeaningOfLife");

  std::cout << "meaning of life:" << theMeaningOfLife << std::endl;
  return 0;
}
