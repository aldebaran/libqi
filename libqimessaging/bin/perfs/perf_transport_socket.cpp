/*
** Author(s):
**  - Laurent LEC <gestes@aldebaran-robotics.com>
**  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/


#include <vector>
#include <iostream>
#include <sstream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <qi/application.hpp>
#include <qimessaging/transportsocket.hpp>

#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/gateway.hpp>

#include <qiperf/dataperfsuite.hpp>

static int gLoopCount = 500;
static const int gThreadCount = 1;
qi::DataPerfSuite* out;

void TSIOnMessageReady(const qi::Message &QI_UNUSED(msg))
{
}

class TSIReply
{
public:
  void onTransportServerNewConnection(qi::TransportSocketPtr socket)
  {
    socket->messageReady.connect(boost::bind<void>(&TSIReply::onMessageReady, this, _1, socket));
    socket->startReading();
  }

  void onMessageReady(const qi::Message &msg, qi::TransportSocketPtr socket)
  {
    qi::Message ret(qi::Message::Type_Reply, msg.address());
    socket->send(ret);
  }
};

int thd_client(qi::TransportSocketPtr socket, const std::string& addr)
{
  if (!socket)
  {
    socket = qi::makeTransportSocket("tcp");

    socket->messageReady.connect(boost::bind<void>(&TSIOnMessageReady, _1));

    socket->connect(addr);
  }

  qi::DataPerf dp;
  qi::Buffer buf;
  char buf2[512];
  buf.write(buf2, 512);

  dp.start("ahah", gLoopCount, 512);
  for (int j = 0; j < gLoopCount; ++j)
  {
    qi::Message msg;
    msg.setBuffer(buf);
    msg.setType(qi::Message::Type_Call);
    msg.setObject(qi::Message::GenericObject_Main);
    msg.setFunction(1);

    if (!(socket->send(msg)))
      return 1;
  }
  dp.stop();
  *out << dp;

  return 0;
}

int main_client(bool shared, unsigned int threadCount, const std::string& addr)
{
  qi::TransportSocketPtr socket;

  boost::thread thd[threadCount < 100 ? threadCount : 100];

  if (shared)
  {
    socket = qi::makeTransportSocket("tcp");
    socket->connect(addr);
    socket->messageReady.connect(boost::bind<void>(&TSIOnMessageReady, _1));
  }

  for (unsigned int i = 0; i < threadCount; ++i)
  {
    thd[i] = boost::thread(boost::bind(&thd_client, socket, addr));
  }

  for (unsigned int i = 0; i < threadCount; ++i)
  {
    thd[i].join();
  }

  return 0;
}

int main_server(const std::string& addr)
{
  TSIReply tsi;
  qi::TransportServer server;

  server.listen(addr);
  server.newConnection.connect(boost::bind(&TSIReply::onTransportServerNewConnection, tsi, _1));
  std::cout << "Now listening on " << server.listenUrl().str() << std::endl;
  while (true)
    qi::os::sleep(60);

  return 0;
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);

  po::options_description desc(std::string("Usage:\n ")+argv[0]);
  desc.add_options()
    ("help,h", "Print this help.")
    ("client-unshared", po::value<std::string>(),
     "Run an unshared client (tcp://x.x.x.x:x)")
    ("client-shared", po::value<std::string>(),
     "Run as a shared client (tcp://x.x.x.x:x)")
    ("server,s", po::value<std::string>()->implicit_value("tcp://0.0.0.0:0"),
     "Run as a server")
    ("thread,t", po::value<unsigned int>()->default_value(1, "1"),
     "How many clients run in the same time")
    ("backend,b", po::value<std::string>()->default_value("normal"),
     "Backend to use to output data (normal | codespeed).")
    ("output,o", po::value<std::string>()->default_value(""),
     "File where output data (if not precise output go to stdout).");

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
            .options(desc).allow_unregistered().run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  qi::DataPerfSuite::OutputType type;
  if (vm["backend"].as<std::string>() == "normal")
    type = qi::DataPerfSuite::OutputType_Normal;
  else if (vm["backend"].as<std::string>() == "codespeed")
    type = qi::DataPerfSuite::OutputType_Codespeed;
  else {
    std::cerr << "This backend doesn't exist, fallback in [normal]!" << std::endl;
    type = qi::DataPerfSuite::OutputType_Normal;
  }

  out = new qi::DataPerfSuite("qimessaging", "perf_transport_socket", type, vm["output"].as<std::string>());

  if (vm.count("client-unshared"))
  {
    std::cout << vm["thread"].as<unsigned int>() << std::endl;
    return main_client(false, vm["thread"].as<unsigned int>(), vm["client-unshared"].as<std::string>());
  }
  else if (vm.count("client-shared"))
  {
    return main_client(true, vm["thread"].as<unsigned int>(), vm["client-shared"].as<std::string>());
  }
  else if (vm.count("server"))
  {
    return main_server(vm["server"].as<std::string>());
  }
  else
  {
    std::cout << desc << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
