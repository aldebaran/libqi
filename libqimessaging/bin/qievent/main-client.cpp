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
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <iostream>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <qimessaging/transport/transport_socket.hpp>
#include <qimessaging/transport/network_thread.hpp>

class RemoteService : public qi::TransportSocketDelegate {
public:
  RemoteService() {
    tc = new qi::TransportSocket();
    tc->setDelegate(this);
  }

  ~RemoteService() {
    tc->disconnect();
    delete tc;
  }

  void connect(const std::string &address,
               unsigned short port)
  {
    tc->connect(address, port, nthd->getEventBase());
    tc->waitForConnected(300);
  }

  void setThread(qi::NetworkThread *n)
  {
    nthd = n;
  }

  void call(const std::string &msg) {
    tc->send(msg);
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
  qi::NetworkThread   *nthd;
  qi::TransportSocket *tc;
};


int main(int argc, char *argv[])
{
  qi::NetworkThread *nthd = new qi::NetworkThread();
  sleep(1);
  RemoteService rs;
  rs.setThread(nthd);
  rs.connect("127.0.0.1", 9559);
  rs.call("call.20.audio.say.hello world!");
  sleep(1);
  rs.call("answer.21.audio.say.what's up!");
  RemoteService rs1;
  rs1.setThread(nthd);
  rs1.connect("127.0.0.1", 9559);
  rs1.call("answer.40.audio.say.hello world!");
  RemoteService rs2;
  rs2.setThread(nthd);
  rs2.connect("127.0.0.1", 9559);
  rs2.call("error.50.audio.say.hello world!");
  rs2.call("event.51.audio.say.hello world!");

  sleep(3);

  delete nthd;

  return 0;

}
