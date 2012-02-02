/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Herve Cuche <hcuche@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/network_client.hpp>
#include <qimessaging/transport.hpp>

namespace qi {

NetworkClient::NetworkClient()
{
  tc = new qi::TransportSocket();
  tc->setDelegate(this);
}

NetworkClient::~NetworkClient()
{
  tc->disconnect();
  delete tc;
}

void NetworkClient::connect(const std::string masterAddress)
{
  tc->connect("127.0.0.1", 5555, nthd->getEventBase());
}

void NetworkClient::setThread(qi::NetworkThread *n)
{
  nthd = n;
}

bool NetworkClient::disconnect()
{
}


bool NetworkClient::waitForConnected(int msecs)
{
  return tc->waitForConnected(msecs);
}

bool NetworkClient::waitForDisconnected(int msecs)
{
  return tc->waitForDisconnected(msecs);
}


void NetworkClient::registerMachine(const qi::MachineInfo& m)
{
}

void NetworkClient::unregisterMachine(const qi::MachineInfo& m)
{
}


bool NetworkClient::isInitialized() const
{
}

std::vector<std::string> NetworkClient::machines() {
  std::vector<std::string> result;

  std::string msg;

  msg = "caca";
  tc->send(msg);
  result.push_back("bim");
  result.push_back("bam");
  return result;
}

void NetworkClient::onConnected(const std::string &msg)
{
  std::cout << "connected: " << msg << std::endl;
}

void NetworkClient::onWrite(const std::string &msg)
{
  std::cout << "written: " << msg << std::endl;
}

void NetworkClient::onRead(const std::string &msg)
{
  std::cout << "read: " << msg << std::endl;
}


}
