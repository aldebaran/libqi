/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <vector>
#include <map>

#include <qimessaging/transport.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/session.hpp>
#include <qi/os.hpp>

#include "qiservicetest.hpp"

namespace qi
{
  ServiceTest::ServiceTest()
  {
    nthd = new qi::NetworkThread();
    ts = new qi::TransportServer();
//    ts->setDelegate(this);
  }

  ServiceTest::~ServiceTest()
  {
    delete ts;
    delete nthd;
  }

  void ServiceTest::start(const std::string &address)
  {
    size_t begin = 0;
    size_t end = 0;
    end = address.find(":");

    std::string ip = address.substr(begin, end);
    begin = end + 1;

    unsigned int port;
    std::stringstream ss(address.substr(begin));
    ss >> port;

    ts->start(ip, port, nthd->getEventBase());
  }

  void ServiceTest::onConnected(const qi::Message &msg)
  {
  }

  void ServiceTest::onWrite(const qi::Message &msg)
  {
  }

  void ServiceTest::onRead(const qi::Message &msg)
  {
    std::cout << "read qiservice: " << msg <<  std::endl;

    if (msg.path() == "reply")
      reply(msg);

  }

  void ServiceTest::reply(const qi::Message &msg)
  {
    qi::Message retval;
    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    retval.setData("msg.data()");

//    ts->send(retval);
  }
}; // !qi
