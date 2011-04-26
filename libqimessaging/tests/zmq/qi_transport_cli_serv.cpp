#include <iostream>

#include <qi/transport.hpp>

// The handler for the service that you want to advertise
int getMeaningOfLife() {
  return 42;
}
const std::string gAddress = "tcp://127.0.0.1:4242";

int main(int argc, char *argv[])
{
  qi::transport::TransportContext ctx;
  qi::transport::TransportServer  ts(ctx);
  qi::transport::TransportClient  tc(ctx);

  //ts.serve(gAddress);
  //ts.run();

  tc.connect(gAddress);

  std::string bufIn;
  std::string bufOut;

  tc.send(bufOut, bufIn);
  // // Connect the client to the master
  // server.connect(masterAddress);
  // server.advertiseService("deepThought.getMeaningOfLife", &getMeaningOfLife);
  // client.connect(masterAddress);

  // int theMeaningOfLife = client.call<int>("deepThought.getMeaningOfLife");
  // return 0;
}
