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
#include <qimessaging/service_info.hpp>
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

std::vector<ServiceInfo> Session::services()
{
  std::vector<ServiceInfo> result;

  qi::Message msg;
  msg.setType(qi::Message::Type_Call);
  msg.setService(qi::Message::Service_ServiceDirectory);
  msg.setPath(qi::Message::Path_Main);
  msg.setFunction(qi::Message::ServiceDirectoryFunction_Services);

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
  qi::Message msg;
  qi::DataStream dr(msg.buffer());
  dr << name;
  msg.setType(qi::Message::Type_Call);
  msg.setService(qi::Message::Service_ServiceDirectory);
  msg.setPath(qi::Message::Path_Main);
  msg.setFunction(qi::Message::ServiceDirectoryFunction_Service);
  tc->send(msg);

  tc->waitForId(msg.id());
  qi::Message ans;
  tc->read(msg.id(), &ans);

  qi::ServiceInfo si;
  qi::DataStream d(ans.buffer());
  d >> si;
  *idx = si.serviceId();

  qi::TransportSocket* ts = NULL;

  //TODO: choose a good endpoint
  qi::Url url(si.endpoints()[0]);

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

  qi::Message msg;
  msg.setType(qi::Message::Type_Call);
  msg.setService(serviceId);
  msg.setPath(qi::Message::Path_Main);
  msg.setFunction(qi::Message::Function_MetaObject);

  ts->send(msg);
  ts->waitForId(msg.id());

  qi::Message ret;
  ts->read(msg.id(), &ret);

  qi::MetaObject *mo = new qi::MetaObject;

  qi::DataStream ds(ret.buffer());

  ds >> *mo;

  qi::RemoteObject *robj = new qi::RemoteObject(ts, serviceId, mo);
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
