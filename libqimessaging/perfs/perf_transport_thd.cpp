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

#include <qimessaging/transport_socket.hpp>
#include <qimessaging/session.hpp>
#include "dataperftimer.hpp"

#include <qimessaging/service_directory.hpp>
#include <qimessaging/gateway.hpp>

static int gLoopCount = 10000;
static const int gThreadCount = 1;
static bool clientDone = false;

int client_calls(qi::Session *session, qi::Object *obj)
{
#if 0
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);
  CPU_SET(1, &mask);
  unsigned int len = sizeof(mask);
  qiLogInfo("sched") << "::" << pthread_setaffinity_np(pthread_self(), len, &mask);
#endif

  if (!obj)
  {
    obj = session->service("serviceTest");

    if (!obj)
    {
      std::cerr << "cant get serviceTest" << std::endl;
      return -1;
    }
  }

  qi::perf::DataPerfTimer dp ("Transport synchronous call");
  for (int i = 0; i < 12; ++i)
  {
    char character = 'c';
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string requeststr = std::string(numBytes, character);

    dp.start(gLoopCount, numBytes);
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
        qiLogInfo("perf_transport_thd") << "error content" << std::endl;
    }
    dp.stop(1);
  }
  return 0;
}


int main_client(bool shared)
{
  const unsigned int nbThreads = 4;
  qi::Session session;
  session.connect("tcp://127.0.0.1:5555");
  qi::Object *obj = 0;
  boost::thread thd[nbThreads];

  qiLogInfo("perf_transport_thd") << "Will spawn " << nbThreads << " threads";

  if (shared)
  {
    qiLogInfo("perf_transport_thd") << "qi::Object will be shared";

    obj = session.service("serviceTest");

    if (!obj)
    {
      std::cerr << "cant get serviceTest" << std::endl;
      return -1;
    }
  }
  else
  {
    qiLogInfo("perf_transport_thd") << "qi::Object won't' be shared";
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

  qi::Session       session;
  qi::Object        obj;
  obj.advertiseMethod("reply", &reply);

  session.connect("tcp://127.0.0.1:5555");

  session.listen("tcp://127.0.0.1:9559");
  session.registerService("serviceTest", &obj);
  std::cout << "serviceTest ready." << std::endl;

  while (!clientDone)
    qi::os::sleep(60);

  return 0;
}

int main(int argc, char **argv)
{
#if 0
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(0, &mask);
  unsigned int len = sizeof(mask);
  qiLogInfo("sched") << "::" << sched_setaffinity(getpid(), len, &mask);
#endif

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
