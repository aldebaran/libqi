
#include <gtest/gtest.h>
#include <string>
#include <boost/timer.hpp>
#include <qi/messaging.hpp>
#include <qi/perf/sleep.hpp>

using namespace qi;

TEST(MasterServerClient, creation)
{
  Master master;
  Server server("server");
  Client client("client");
}

TEST(ClientTest, creation)
{
  Client client("client");
}

TEST(ClientTest, multipleCreation)
{
  for (int i = 0; i< 100; i++) {
    Client client("client");
  }
}



//TEST(ServerTest, creation)
//{
//  Server server(gServerName, gServerAddress, gMasterAddress);
//  Sleep(1000);
//}

//TEST(ServerTest, multipleCreationNewPorts)
//{
//  // PROBLEM: limited by number of threads !?
//
//  int numServers = 5; // at 12 it crashes
//  for (int i = 5560; i< 5560 + numServers; i++) {
//
//    std::stringstream ss;
//    ss << "tcp://127.0.0.1:";
//    ss << i;
//    std::string serverAddress = ss.str();
//    std::cout << "Creating Server " << serverAddress << std::endl;
//    Server server(gServerName, gMasterAddress);
//    // after about 12 services ... crash
//    // not enough storage space is available ../src/thread.cpp
//    // sometimes a pthread create error.
//    sleep(1);
//    std::cout << "Created Server " << serverAddress << std::endl;
//  }
//}

TEST(ServerTest, multipleCreationSamePort)
{
  // PROBLEM: starting servers too fast crashes
  // PROBLEM: server object should throw, or needs OK? method
  for (int i = 0; i< 10; i++) {
    std::cout << "Creating Server " << i << std::endl;
    Server server;
    // without this sleep, bad things happen!!!
    sleep(1);
    std::cout << "Created Server " << i << std::endl;
  }
}




int test(const int &t)
{
  return t + 42;
}
