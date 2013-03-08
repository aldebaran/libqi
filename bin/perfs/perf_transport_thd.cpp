/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/


#include <vector>
#include <iostream>
#include <sstream>
#if 0
#include <sched.h>
#endif
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include "transportsocket.hpp"
#include <qimessaging/session.hpp>
#include <qiperf/dataperfsuite.hpp>

#include <qi/application.hpp>
#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/gateway.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>

qiLogCategory("test");

static int gLoopCount = 10000;
static const int gThreadCount = 1;
static bool clientDone = false;

int client_calls(qi::Session *session, qi::ObjectPtr obj)
{
  if (!obj)
  {
    obj = session->service("serviceTest");

    if (!obj)
    {
      std::cerr << "cant get serviceTest" << std::endl;
      return -1;
    }
  }

  qi::DataPerf dp;
  for (int i = 0; i < 12; ++i)
  {
    char character = 'c';
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string requeststr = std::string(numBytes, character);

    dp.start("ohoh", gLoopCount, numBytes);
    for (int j = 0; j < gLoopCount; ++j)
    {
      static int id = 1;
      id++;
      char c = 1;
      c++;
      if (c == 0)
        c++;
      requeststr[2] = c;

      std::string result = obj->call<std::string>("reply", requeststr);
      if (result != requeststr)
        qiLogInfo() << "error content" << std::endl;
    }
    dp.stop();
  }
  return 0;
}


int main_client(bool shared)
{
  const unsigned int nbThreads = 4;
  qi::Session session;
  session.connect("tcp://127.0.0.1:5555");
  qi::ObjectPtr obj;
  boost::thread thd[nbThreads];

  qiLogInfo() << "Will spawn " << nbThreads << " threads";

  if (shared)
  {
    qiLogInfo() << "qi::GenericObject will be shared";

    obj = session.service("serviceTest");

    if (!obj)
    {
      std::cerr << "cant get serviceTest" << std::endl;
      return -1;
    }
  }
  else
  {
    qiLogInfo() << "qi::GenericObject won't' be shared";
  }

  for (unsigned int i = 0; i < nbThreads; i++)
  {
    thd[i] = boost::thread(boost::bind(&client_calls, &session, obj));
  }

  for (unsigned int i = 0; i < nbThreads; i++)
  {
    thd[i].join();
  }
  clientDone = true;
  return 0;
}

void start_client(int count, bool shared)
{
  boost::thread thd[100];

  assert(count < 100);

  for (int i = 0; i < count; ++i)
  {
    std::stringstream ss;
    ss << "remote" << i;
    std::cout << "starting thread: " << ss.str() << std::endl;
    thd[i] = boost::thread(boost::bind(&main_client, shared));
  }

  for (int i = 0; i < count; ++i)
    thd[i].join();
}


#include <qi/os.hpp>


int main_gateway()
{
  qi::Gateway       gate;

  gate.attachToServiceDirectory("tcp://127.0.0.1:5555");
  gate.listen("tcp://127.0.0.1:12345");
  std::cout << "ready." << std::endl;
  while (!clientDone)
    qi::os::sleep(60);

  return 0;
}

std::string reply(const std::string &msg)
{
  return msg;
}

int main_server()
{
  qi::ServiceDirectory sd;
  sd.listen("tcp://127.0.0.1:5555");
  std::cout << "Service Directory ready." << std::endl;

  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);

  qi::Session       session;
  qi::ObjectPtr obj(ob.object());

  session.connect("tcp://127.0.0.1:5555");

  session.listen("tcp://127.0.0.1:9559");
  session.registerService("serviceTest", obj);
  std::cout << "serviceTest ready." << std::endl;

  while (!clientDone)
    qi::os::sleep(60);

  return 0;
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);

  std::string usage = "If no mode is specified, run client and server in same process\n" \
                      "Environment used: VALGRIND, NO_GATEWAY, SYNCHRONOUS\n";

  po::options_description desc(std::string("Usage:\n ")+argv[0]+"\n" + usage);
  desc.add_options()
    ("help,h", "Print this help.")
    ("all", po::value<std::string>()->implicit_value("tcp://0.0.0.0:0"), "(default) Run all in the same process.")
    ("client,c", po::value<std::string>(), "Run as a client (tcp://xxx.xxx.xxx.xxx:xxxxx).")
    ("server,s", po::value<std::string>()->implicit_value("tcp://0.0.0.0:0"), "Run as a server.")
    ("gateway", po::value<std::string>(), "Run as a gateway.")
    ("local", "Run in local.")
    ("thread", po::value<int>()->default_value(1, "1"),
     "Number of thread to launch for clients")
    ("backend", po::value<std::string>()->default_value("normal"),
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

  if (argc > 1 && !strcmp(argv[1], "--client-shared"))
  {
    int threadc = 1;
    if (argc > 2)
      threadc = atoi(argv[2]);
    start_client(threadc, true);
  }
  else if (argc > 1 && !strcmp(argv[1], "--client-unshared"))
  {
    int threadc = 1;
    if (argc > 2)
      threadc = atoi(argv[2]);
    start_client(threadc, false);
  }
  else if (argc > 1 && !strcmp(argv[1], "--server"))
  {
    return main_server();
  }
  else if (argc > 1 && !strcmp(argv[1], "--gateway"))
  {
    return main_gateway();
  }
  else
  {
    //start the server
    boost::thread threadServer1(boost::bind(&main_server));
    qi::os::sleep(1);
    boost::thread threadServer2(boost::bind(&main_gateway));
    qi::os::sleep(1);
    start_client(gThreadCount, false);
  }
  return 0;
}
