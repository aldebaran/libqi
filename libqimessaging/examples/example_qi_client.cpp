/**
 * This is a simple QiMessaging example
 */

#include <iostream>
#include <qimessaging/client.hpp>

int main()
{
  // Create a client
  qi::Client client("myClient");

  // Connect the client to the master
  const std::string masterAddress = "127.0.0.1:5555";
  client.connect(masterAddress);

  // Call a method, expecting an int return type
  int theMeaningOfLife = client.call<int>("deepThought.getMeaningOfLife");

  std::cout << "Meaning of life:" << theMeaningOfLife << std::endl;

  return 0;
}
