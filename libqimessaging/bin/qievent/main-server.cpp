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

#include "network-thread.hpp"
#include "transport-server.hpp"


class RemoteServer : public TransportServerDelegate
{
public:
  RemoteServer()
  {
    ts = new TransportServer();
    ts->setDelegate(this);
  }

  ~RemoteServer()
  {
    delete ts;
  }

  void setThread(NetworkThread *n)
  {
    nthd = n;
  }

  void start(const std::string &address,
             unsigned short port)
  {
    ts->start(address, port, nthd->getEventBase());
  }

  virtual void onConnected(const std::string &msg)
  {
    std::cout << "connected: " << msg << std::endl;
  }

  virtual void onWrite(const std::string &msg)
  {
    std::cout << "written: " << msg << std::endl;
  }

  virtual void onRead(const std::string &msg)
  {
    std::cout << "read: " << msg << std::endl;
  }

private:
  NetworkThread   *nthd;
  TransportServer *ts;
};

int main(int argc, char *argv[])
{
  NetworkThread *nthd = new NetworkThread();
  sleep(1);
  RemoteServer rs;
  rs.setThread(nthd);
  rs.start("127.0.0.1", 9559);

  while (true)
    ;

  delete nthd;

  return 0;
}
