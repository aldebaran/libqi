/*
** Author(s):
**  - Laurent LEC <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/


#include <vector>
#include <iostream>
#include <sstream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <qimessaging/transport_socket.hpp>
#include <qimessaging/session.hpp>
#include "dataperftimer.hpp"

#include <qimessaging/service_directory.hpp>
#include <qimessaging/gateway.hpp>

static int gLoopCount = 10000;
static const int gThreadCount = 1;

class TSIRead: public qi::TransportSocketInterface
{
  void onSocketReadyRead(qi::TransportSocket *client, int id, void *data)
  {
    qi::Message msg;
    client->read(id, &msg);
  }
};

class TSIReply: public qi::TransportSocketInterface,
                public qi::TransportServerInterface
{
  void onTransportServerNewConnection(qi::TransportServer* server, qi::TransportSocket *socket, void *data)
  {
    socket->addCallbacks(this);
  }

  void onSocketReadyRead(qi::TransportSocket *client, int id, void *data)
  {
    qi::Message msg;
    client->read(id, &msg);
    qi::Message ret;
    ret.buildReplyFrom(msg);
    client->send(ret);
  }
};

int thd_client(qi::TransportSocket *socket)
{
  qi::Session session;
  qi::TransportSocket ts;
  TSIRead tsi;

  if (!socket)
  {
    ts.connect("tcp://127.0.0.1:5555");
    ts.waitForConnected(30);
    socket = &ts;
    socket->addCallbacks(&tsi);
  }

  qi::perf::DataPerfTimer dp ("Transport synchronous call");
  qi::Buffer buf;
  char buf2[512];
  buf.write(buf2, 512);
  qi::Message msg;
  msg.setBuffer(buf);
  msg.setType(qi::Message::Type_Call);
  msg.setObject(qi::Message::Object_Main);
  msg.setFunction(1);

  for (int i = 0; i < 12; ++i)
  {
    dp.start(gLoopCount, 512);
    for (int j = 0; j < gLoopCount; ++j)
    {
      socket->send(msg);
    }
    dp.stop(1);
  }
  return 0;
}

int main_client(bool shared)
{
  const int count = 4;
  qi::TransportSocket socket;
  qi::Session session;
  TSIRead tsi;
  boost::thread thd[count];

  if (shared)
  {
    qiLogInfo("perf_transport_socket") << "Socket will be shared";
    socket.connect("tcp://127.0.0.1:5555");
    socket.waitForConnected(30);
    socket.addCallbacks(&tsi);
  }
  else
  {
    qiLogInfo("perf_transport_socket") << "Socket won't be shared";
  }

  for (int i = 0; i < count; ++i)
  {
    thd[i] = boost::thread(boost::bind(&thd_client, shared ? &socket : 0));
  }

  for (int i = 0; i < count; ++i)
  {
    thd[i].join();
  }

  return 0;
}

int main_server()
{
  TSIReply tsi;
  qi::TransportServer server;
  qi::Session session;
  server.listen("tcp://127.0.0.1:5555");
  server.addCallbacks(&tsi);
  qiLogInfo("server") << "Now listening on tcp://127.0.0.1:5555";
  while (true)
    qi::os::sleep(60);

  return 0;
}

int main(int argc, char **argv)
{
  if (argc > 1 && !strcmp(argv[1], "--client-unshared"))
  {
    return main_client(false);
  }
  else if (argc > 1 && !strcmp(argv[1], "--client-shared"))
  {
    return main_client(true);
  }
  else if (argc > 1 && !strcmp(argv[1], "--server"))
  {
    return main_server();
  }
  else
  {
    std::cout << "Usage: " << argv[0]
              << " --{client-{shared,unshared},server}" << std::endl;
  }
  return 0;
}
