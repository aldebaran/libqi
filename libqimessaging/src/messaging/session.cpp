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

static int uniqueRequestId = 0;

namespace qi {

class RemoteObject : public qi::Object {
public:
  explicit RemoteObject(qi::TransportSocket *ts, const std::string &dest)
    : _ts(ts),
      _dest(dest)
  {
  }

  virtual void metaCall(const std::string &method, const std::string &sig, DataStream &in, DataStream &out) {

    qi::Message msg;
    msg.setId(uniqueRequestId++);
    msg.setSource("ouame");
    msg.setDestination(_dest);
    msg.setPath(method);
    msg.setData(in.str());

    _ts->send(msg);
    _ts->waitForId(msg.id());

    qi::Message ret;
    _ts->read(msg.id(), &ret);
    out.str(ret.data());
  }

protected:
  qi::TransportSocket *_ts;
  std::string _dest;
};


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

void Session::registerEndpoint(const std::string &e)
{
  qi::DataStream d;
  qi::EndpointInfo endpoint;
  size_t begin = 0;
  size_t end = 0;
  end = e.find(":");
  endpoint.type = e.substr(begin, end);
  begin = end + 3;
  end = e.find(":", begin);
  endpoint.ip = e.substr(begin, end - begin);
  begin = end + 1;
  std::stringstream ss(e.substr(begin));
  ss >> endpoint.port;
  d << endpoint;

  qi::Message msg;
  msg.setId(uniqueRequestId++);
  msg.setSource(_name);
  msg.setPath("registerEndpoint");
  msg.setData(d.str());

  tc->send(msg);
  tc->waitForId(msg.id());
  qi::Message ans;
  tc->read(msg.id(), &ans);
}

void Session::unregisterEndpoint(const std::string &e)
{
  qi::DataStream d;
  qi::EndpointInfo endpoint;
  size_t begin = 0;
  size_t end = 0;
  end = e.find(":");
  endpoint.type = e.substr(begin, end);
  begin = end + 3;
  end = e.find(":", begin);
  endpoint.ip = e.substr(begin, end - begin);
  begin = end + 1;
  std::stringstream ss(e.substr(begin));
  ss >> endpoint.port;
  d << endpoint;

  qi::Message msg;
  msg.setId(uniqueRequestId++);
  msg.setSource(_name);
  msg.setPath("unregisterEndpoint");
  msg.setData(d.str());

  tc->send(msg);
  tc->waitForId(msg.id());
  qi::Message ans;
  tc->read(msg.id(), &ans);
}

std::vector<std::string> Session::services()
{
  std::vector<std::string> result;

  qi::Message msg;
  msg.setId(uniqueRequestId++);
  msg.setSource(_name);
  msg.setPath("services");

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
  std::vector<qi::EndpointInfo> result;

  qi::Message msg;

  msg.setId(uniqueRequestId++);
  msg.setSource(_name);
  msg.setPath("service");
  msg.setData(name);

  tc->send(msg);

  tc->waitForId(msg.id());
  qi::Message ans;
  tc->read(msg.id(), &ans);

  qi::DataStream d(ans.data());
  d >> result;

  qi::TransportSocket* ts = NULL;
  std::vector<qi::EndpointInfo>::iterator endpointIt;
  for (endpointIt = result.begin(); endpointIt != result.end(); ++endpointIt)
  {
    if (endpointIt->type == type)
    {
      ts = new qi::TransportSocket();
      ts->setDelegate(this);
      ts->connect(endpointIt->ip, endpointIt->port, _nthd->getEventBase());
      ts->waitForConnected();
    }
  }

  return ts;
}


qi::Object* Session::service(const std::string &name,
                             const std::string &type)
{
  qi::Object          *obj;
  qi::TransportSocket *ts = serviceSocket(name, type);

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
