/*
** main-client.cpp
** Login : <hcuche@hcuche-de>
** Started on  Tue Jan  3 13:52:00 2012 Herve Cuche
** $Id$
**
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Herve Cuche
*/

#include <qimessaging/transport_server.hpp>
#include <qimessaging/network_thread.hpp>

class RemoteServer : public qi::TransportServer
{
public:
  RemoteServer()
  {
    nthd = NULL;
    ts = new qi::TransportServer();
//    ts->setCallbacks(this);
  }

  ~RemoteServer()
  {
    delete ts;
  }

  void setThread(qi::NetworkThread *n)
  {
    nthd = n;
  }

  void start(const std::string &address,
             unsigned short port)
  {
    ts->start(address, port, nthd->getEventBase());
  }

  virtual void onSocketConnected(const qi::Message &msg)
  {
    std::cout << "connected: " << msg.str() << std::endl;
  }

  virtual void onWrite(const qi::Message &msg)
  {
    std::cout << "written: " << msg.str() << std::endl;
  }

  virtual void onRead(const qi::Message &msg)
  {
    std::cout << "read: " << msg.str() << std::endl;
  }

private:
  qi::NetworkThread   *nthd;
  qi::TransportServer *ts;
};

int main(int argc, char *argv[])
{
  qi::NetworkThread *nthd = new qi::NetworkThread();
  sleep(1);
  RemoteServer rs;
  rs.setThread(nthd);
  rs.start("127.0.0.1", 9559);

  while (true)
    ;

  delete nthd;

  return 0;
}
