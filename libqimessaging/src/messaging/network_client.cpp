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

  qi::Message msg;

  msg.setId(0);
  msg.setSource("moi");
  msg.setDestination("qi.servicedirectorymanager");
  msg.setPath("machines");
  tc->send(msg);
  tc->waitForId(msg.id());
  qi::Message ans;
  tc->read(&msg);
  return result;
}

void NetworkClient::onConnected(const qi::Message &msg)
{
  std::cout << "connected: " << std::endl;
}

void NetworkClient::onWrite(const qi::Message &msg)
{
  std::cout << "written: " << std::endl;
}

void NetworkClient::onRead(const qi::Message &msg)
{
  std::cout << "read: " << std::endl;
}


}
