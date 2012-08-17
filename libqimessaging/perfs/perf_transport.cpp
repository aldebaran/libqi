/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/


#include <vector>
#include <iostream>
#include <sstream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>

#include <qi/application.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/session.hpp>
#include "dataperftimer.hpp"

#include <qimessaging/service_directory.hpp>
#include <qimessaging/gateway.hpp>

static int gLoopCount = getenv("VALGRIND")?500:10000;
static const int gThreadCount = 1;
static bool allInOne = false; // True if sd/server/client are in this process
static std::string sdPort;
static bool clientDone = false;
int run_client(qi::Object* obj);

std::string reply(const std::string &msg)
{
  return msg;
}

int main_local()
{
  qi::Object        obj;
  obj.advertiseMethod("reply", &reply);
  run_client(&obj);
  return 0;
}

int main_client(std::string QI_UNUSED(src), std::string host, std::string port)
{
  if (host.empty())
    host = "127.0.0.1";
  if (port.empty())
    port = allInOne?sdPort:std::string("5555");


  qi::Session session;
  session.connect("tcp://"+host+":"+port);
  session.waitForConnected();
  qi::Future< qi::Object * > fobj =  session.service("serviceTest");
  qi::Object *obj = fobj.value();
  if (!obj)
  {
    std::cerr << "cant get serviceTest" << std::endl;
    return -1;
  }
  return run_client(obj);
}

int run_client(qi::Object* obj)
{
  qi::perf::DataPerfTimer dp ("Transport synchronous call");
  for (int i = 0; i < 12; ++i)
  {
    char character = 'c';
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string requeststr = std::string(numBytes, character);

    dp.start(gLoopCount, numBytes);
    unsigned long long latencySum = 0;
    for (int j = 0; j < gLoopCount; ++j)
    {
      static int id = 1;
      id++;
      char c = 1;
      c++;
      if (c == 0)
        c++;
      requeststr[2] = c;
      qi::os::timeval tstart, tstop;
      qi::os::gettimeofday(&tstart);
      std::string result = obj->call<std::string>("reply", requeststr);
      qi::os::gettimeofday(&tstop);
      latencySum += (tstop.tv_sec - tstart.tv_sec)* 1000000LL
      + (tstop.tv_usec - tstart.tv_usec);
      if (result != requeststr)
        std::cout << "error content" << std::endl;
    }
    dp.stop(1);
    // We expect latency to be dp.meanInterval, but just to be sure also show
    // latency.
    std::cerr << "Average latency " << (latencySum / gLoopCount) << std::endl;
  }
  return 0;
}

void start_client(int count, std::string host, std::string port)
{
  boost::thread thd[100];

  assert(count < 100);

  for (int i = 0; i < count; ++i)
  {
    std::stringstream ss;
    ss << "remote" << i;
    std::cout << "starting thread: " << ss.str() << std::endl;
    thd[i] = boost::thread(boost::bind(&main_client, ss.str(), host, port));
  }

  for (int i = 0; i < count; ++i)
    thd[i].join();
  clientDone = true;
}


#include <qi/os.hpp>


int main_gateway(std::string host, std::string port)
{
  if (host.empty())
    host = "127.0.0.1";
  if (port.empty())
    port = allInOne?sdPort:std::string("5555");

  qi::Gateway       gate;

  gate.attachToServiceDirectory("tcp://"+host+":"+port);
  gate.listen("tcp://0.0.0.0:12345");
  std::cout << "ready." << std::endl;
  while (!clientDone)
    qi::os::sleep(60);

  return 0;
}

int main_server(std::string host, std::string port)
{
  if (port.empty())
    port = "0";
  if (host.empty())
    host = "0.0.0.0";
  qi::ServiceDirectory sd;
  sd.listen("tcp://" + host +":" + port);
  if (port == "0")
    port = boost::lexical_cast<std::string>(sd.listenUrl().port());
  sdPort = port;
  std::cout << "Service Directory ready on " << sd.listenUrl().str() << std::endl;

  qi::Session       session;
  qi::Object        obj;
  obj.advertiseMethod("reply", &reply);

  session.connect("tcp://127.0.0.1:"+port);
  session.waitForConnected();

  session.listen("tcp://0.0.0.0:0");
  session.registerService("serviceTest", &obj);
  std::cout << "serviceTest ready." << std::endl;

  while (!clientDone)
    qi::os::sleep(60);
  return 0;
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);
  if (argc > 1
    && (argv[1] == std::string("--help") || argv[1] == std::string("-h")))
  {
    std::cerr << argv[0]
      << "[--client threadcount | --server | --gateway] [sdHost] [sdPort]\n"
      << "\t If no mode is specified, run client and server in same process"
      << std::endl;
    exit(1);
  }
  std::string host, port;
  if (argc > 2)
    host = argv[2];
  if (argc > 3)
    port = argv[3];
  if (argc > 1 && !strcmp(argv[1], "--client"))
  {
    int threadc = 1;
    if (argc > 2)
      threadc = atoi(argv[2]);
    if (argc > 3)
      host = argv[3];
    if (argc > 4)
    port = argv[4];
    start_client(threadc, host, port);
  }
  else if (argc > 1 && !strcmp(argv[1], "--server"))
  {
    return main_server(host, port);
  }
  else if (argc > 1 && !strcmp(argv[1], "--gateway"))
  {
    return main_gateway(host, port);
  }
  else if (argc > 1 && !strcmp(argv[1], "--local"))
  {
    return main_local();
  }
  else
  {
    //start the server
    allInOne = true;
    boost::thread threadServer1(boost::bind(&main_server, host, port));
    qi::os::msleep(500); // give it time to listen
    boost::thread threadServer2(boost::bind(&main_gateway, host, port));
    qi::os::sleep(1);
    start_client(gThreadCount, host, "12345");
  }
  return 0;
}
