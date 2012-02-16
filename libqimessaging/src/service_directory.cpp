/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iostream>
#include <vector>
#include <map>

#include <qimessaging/transport.hpp>
#include <qimessaging/service_directory.hpp>
#include <qimessaging/session.hpp>
#include <qimessaging/datastream.hpp>
#include <qi/os.hpp>


namespace qi
{
class ServiceDirectoryPrivate : public TransportServerInterface, public TransportSocketInterface
{
public:
  ServiceDirectoryPrivate()
  {
    nthd = new qi::NetworkThread();
    ts = new qi::TransportServer();
    ts->setDelegate(this);
  }

  ~ServiceDirectoryPrivate()
  {
    delete ts;
    delete nthd;
  }

  virtual void newConnection()
  {
    TransportSocket *socket = ts->nextPendingConnection();
    if (!socket)
      return;
    socket->setDelegate(this);
  }

  virtual void onReadyRead(TransportSocket *socket, const qi::Message &msg)
  {
    std::cout << msg.function() << std::endl;
    qi::Message retval;
    if (msg.function() == "services")
      services(msg, retval);
    else if (msg.function() == "service")
      service(msg, retval);
    else if (msg.function() == "registerService")
      registerService(msg, retval);
    else
      return;
    socket->send(retval);
  }

  virtual void onWriteDone(TransportSocket *client)
  {
  }

  virtual void onConnected(TransportSocket *client)
  {
  }

  virtual void onDisconnected(TransportSocket *client)
  {
  }

  // signature: services()
  void services(const qi::Message &msg, qi::Message &retval)
  {
    std::vector<std::string> result;

    std::map<std::string, std::string>::iterator it;
    for (it = connectedServices.begin(); it != connectedServices.end(); ++it)
      result.push_back(it->first);

    qi::DataStream d;
    d << result;

    retval.setType(qi::Message::Reply);
    retval.setId(msg.id());
    retval.setData(d.str());
  }


  // signature: service(name)
  void service(const qi::Message &msg, qi::Message &retval)
  {
    qi::DataStream din(msg.data());
    std::string name;
    din >> name;
    qi::DataStream d;
    std::map<std::string, std::string>::iterator servicesIt;
    std::vector<std::string> result;
    result.push_back(name);
    servicesIt = connectedServices.find(name);
    if (servicesIt != connectedServices.end())
    {
      result.push_back(servicesIt->second);
    }

    d << result;

    retval.setType(qi::Message::Reply);
    retval.setId(msg.id());
    retval.setData(d.str());
  }

  // signature: registerService(string name, string url)
  void registerService(const qi::Message &msg, qi::Message &retval)
  {
    std::string name;
    std::string url;
    qi::DataStream d(msg.data());
    d >> name;
    d >> url;

    connectedServices[name] = url;

    retval.setType(qi::Message::Reply);
    retval.setId(msg.id());
    retval.setData(msg.source() + " register.");
  }

public:
  qi::NetworkThread                     *nthd;
  qi::TransportServer                   *ts;
  std::map<std::string, std::string>     connectedServices;

}; // !ServiceDirectoryPrivate


ServiceDirectory::ServiceDirectory()
  : _p(new ServiceDirectoryPrivate())
{
}

ServiceDirectory::~ServiceDirectory()
{
  delete _p;
}

void ServiceDirectory::start(const std::string &address)
{
  size_t begin = 0;
  size_t end = 0;
  end = address.find(":");

  std::string ip = address.substr(begin, end);
  begin = end + 1;

  unsigned int port;
  std::stringstream ss(address.substr(begin));
  ss >> port;

  _p->ts->start(ip, port, _p->nthd->getEventBase());
}

}; // !qi
