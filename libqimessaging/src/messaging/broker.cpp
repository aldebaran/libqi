/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Herve Cuche <hcuche@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/broker.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/transport.hpp>

static int uniqueRequestId = 0;

namespace qi {

Broker::Broker()
{
  tc = new qi::TransportSocket();
  tc->setDelegate(this);
}

Broker::~Broker()
{
  tc->disconnect();
  delete tc;
}

void Broker::connect(const std::string &masterAddress)
{
  size_t begin = 0;
  size_t end = 0;
  end = masterAddress.find(":");

  std::string ip = masterAddress.substr(begin, end);
  begin = end + 1;

  unsigned int port;
  std::stringstream ss(masterAddress.substr(begin));
  ss >> port;

  tc->connect(ip, port, nthd->getEventBase());
}

void Broker::setThread(qi::NetworkThread *n)
{
  nthd = n;
}

bool Broker::disconnect()
{
  return true;
}


bool Broker::waitForConnected(int msecs)
{
  return tc->waitForConnected(msecs);
}

bool Broker::waitForDisconnected(int msecs)
{
  return tc->waitForDisconnected(msecs);
}


void Broker::registerMachine(const qi::MachineInfo& m)
{
}

void Broker::unregisterMachine(const qi::MachineInfo& m)
{
}


bool Broker::isInitialized() const
{
  return true;
}

std::vector<std::string> Broker::machines()
{
  std::vector<std::string> result;

  qi::Message msg;
  msg.setId(uniqueRequestId++);
  msg.setSource(_name);
  msg.setDestination("qi.servicedirectorymanager");
  msg.setPath("machines");

  tc->send(msg);
  tc->waitForId(msg.id());
  qi::Message ans;
  tc->read(msg.id(), &ans);

  return result;
}


  return result;
}



void Broker::onConnected(const qi::Message &msg)
{
//  std::cout << "connected broker: " << std::endl;
}

void Broker::onWrite(const qi::Message &msg)
{
//  std::cout << "written broker: " << std::endl;
}

void Broker::onRead(const qi::Message &msg)
{
//  std::cout << "read broker: " << std::endl;
}


}
