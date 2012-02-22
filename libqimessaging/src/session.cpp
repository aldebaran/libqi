/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Herve Cuche <hcuche@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/object.hpp>
#include "src/url.hpp"
#include "src/remoteobject_p.hpp"
#include "src/network_thread.hpp"

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
  qi::Url url(masterAddress);

  tc->connect(url.host(), url.port(), _nthd->getEventBase());
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
  msg.setService(qi::Message::ServiceDirectory);
  msg.setPath(0);
  msg.setFunction(qi::Message::Services);

  tc->send(msg);

  tc->waitForId(msg.id());
  qi::Message ans;
  tc->read(msg.id(), &ans);

  qi::DataStream d(ans.buffer());
  d >> result;

  return result;
}

qi::TransportSocket* Session::serviceSocket(const std::string &name,
                                            unsigned int      *idx,
                                            const std::string &type)
{
  std::vector<std::string> result;
  qi::Message msg;
  qi::DataStream dr(msg.buffer());
  dr << name;
  msg.setType(qi::Message::Call);
  msg.setService(qi::Message::ServiceDirectory);
  msg.setPath(0);
  msg.setFunction(qi::Message::Service);
  tc->send(msg);

  tc->waitForId(msg.id());
  qi::Message ans;
  tc->read(msg.id(), &ans);

  qi::DataStream d(ans.buffer());
  d >> result;
  std::stringstream ss(result[0]);
  ss >> *idx;

  qi::TransportSocket* ts = NULL;

  qi::Url url(result[1]);

  ts = new qi::TransportSocket();
  ts->setDelegate(this);
  ts->connect(url.host(), url.port(), _nthd->getEventBase());
  ts->waitForConnected();

  return ts;
}


qi::Object* Session::service(const std::string &service,
                             const std::string &type)
{
  qi::Object          *obj;
  unsigned int serviceId = 0;
  qi::TransportSocket *ts = serviceSocket(service, &serviceId, type);

  if (ts == 0)
  {
    return 0;
  }

  qi::RemoteObject *robj = new qi::RemoteObject(ts, serviceId);
  obj = robj;
  return obj;
}

void Session::join()
{
  _nthd->join();
}

void Session::onConnected(TransportSocket *client)
{
  //  std::cout << "connected broker: " << std::endl;
}

void Session::onWriteDone(TransportSocket *client)
{
  //  std::cout << "written broker: " << std::endl;
}

void Session::onReadyRead(TransportSocket *client, Message &msg)
{
  //  std::cout << "read broker: " << std::endl;
}

}
