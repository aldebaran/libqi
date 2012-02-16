/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Herve Cuche <hcuche@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/transport.hpp>
#include <qimessaging/object.hpp>
#include "src/remoteobject_p.hpp"

static int uniqueRequestId = 0;

namespace qi {



Session::Session()
{
  _nthd = new qi::NetworkThread();

  tc = new qi::TransportSocket();
  tc->setDelegate(this);
}

Session::~Session()
{
  tc->disconnect();
  delete tc;
}

void Session::connect(const std::string &masterAddress)
{
  size_t begin = 0;
  size_t end = 0;
  end = masterAddress.find(":");

  std::string ip = masterAddress.substr(begin, end);
  begin = end + 1;

  unsigned int port;
  std::stringstream ss(masterAddress.substr(begin));
  ss >> port;

  tc->connect(ip, port, _nthd->getEventBase());
}

bool Session::disconnect()
{
  return true;
}

void Session::onDisconnected(TransportSocket *client)
{
}


bool Session::waitForConnected(int msecs)
{
  return tc->waitForConnected(msecs);
}

bool Session::waitForDisconnected(int msecs)
{
  return tc->waitForDisconnected(msecs);
}

std::vector<std::string> Session::services()
{
  std::vector<std::string> result;

  qi::Message msg;
  msg.setType(qi::Message::Call);
  msg.setSource(_name);
  msg.setFunction("services");

  tc->send(msg);

  tc->waitForId(msg.id());
  qi::Message ans;
  tc->read(msg.id(), &ans);

  qi::DataStream d(ans.data());
  d >> result;

  return result;
}

qi::TransportSocket* Session::serviceSocket(const std::string &name,
                                            const std::string &type)
{
  std::string result;
  qi::DataStream dr;
  qi::Message msg;
  dr << name;
  msg.setId(uniqueRequestId++);
  msg.setFunction("service");
  msg.setData(dr.str());

  tc->send(msg);

  tc->waitForId(msg.id());
  qi::Message ans;
  tc->read(msg.id(), &ans);

  qi::DataStream d(ans.data());
  d >> result;

  qi::TransportSocket* ts = NULL;

  qi::EndpointInfo endpoint;
  size_t begin = 0;
  size_t end = 0;
  end = result.find(":");
  endpoint.type = result.substr(begin, end);
  begin = end + 3;
  end = result.find(":", begin);
  endpoint.ip = result.substr(begin, end - begin);
  begin = end + 1;
  std::stringstream ss(result.substr(begin));
  ss >> endpoint.port;

  ts = new qi::TransportSocket();
  ts->setDelegate(this);
  ts->connect(endpoint.ip, endpoint.port, _nthd->getEventBase());
  ts->waitForConnected();

  return ts;
}


qi::Object* Session::service(const std::string &name,
                             const std::string &type)
{
  qi::Object          *obj;
  qi::TransportSocket *ts = serviceSocket(name, type);

  if (ts == 0)
  {
    return 0;
  }

  qi::RemoteObject *robj = new qi::RemoteObject(ts, name);
  obj = robj;
  return obj;
}

void Session::onConnected(TransportSocket *client)
{
  //  std::cout << "connected broker: " << std::endl;
}

void Session::onWriteDone(TransportSocket *client)
{
  //  std::cout << "written broker: " << std::endl;
}

void Session::onReadyRead(TransportSocket *client, const Message &msg)
{
  //  std::cout << "read broker: " << std::endl;
}

}
