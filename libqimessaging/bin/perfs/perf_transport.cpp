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
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <qi/application.hpp>
#include <qimessaging/transport_socket.hpp>
#include <qimessaging/session.hpp>
#include "dataperftimer.hpp"

#include <qimessaging/service_directory.hpp>
#include <qimessaging/gateway.hpp>
#include <qimessaging/object.hpp>
#include <qimessaging/objectbuilder.hpp>

static int gLoopCount = getenv("VALGRIND")?500:10000;
static const int gThreadCount = 1;
static bool allInOne = false; // True if sd/server/client are in this process
static std::string sdPort;
static bool clientDone = false;
static bool serverReady = false;
int run_client(qi::Object* obj);

std::string reply(const std::string &msg)
{
  return msg;
}

qi::Buffer replyBuf(const qi::Buffer& buf)
{
  return qi::Buffer(buf);
}

int main_local()
{
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  ob.advertiseMethod("replyBuf", &replyBuf);
  qi::Object obj(ob.object());
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
  qi::Object obj =  session.service("serviceTest");
  if (!obj.isValid())
  {
    std::cerr << "cant get serviceTest" << std::endl;
    return -1;
  }
  return run_client(&obj);
}


int run_client(qi::Object* obj)
{
  qi::perf::DataPerfTimer dp ("Transport synchronous call");
  int rstart = 0;
  if (getenv("rstart")) rstart = strtol(getenv("rstart"), 0, 0);
  int rend = 20;
  if (getenv("rend")) rend = strtol(getenv("rend"), 0, 0);
  for (int i = rstart; i < rend; i+=2)
  {
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    qi::Buffer buf;
    buf.reserve(numBytes);
    dp.start(gLoopCount, numBytes);
    unsigned long long latencySum = 0;
    for (int j = 0; j < gLoopCount; ++j)
    {
      static int id = 1;
      id++;
      qi::os::timeval tstart, tstop;
      qi::os::gettimeofday(&tstart);
      qi::Buffer result = obj->call<qi::Buffer>("replyBuf", buf);
      qi::os::gettimeofday(&tstop);
      latencySum += (tstop.tv_sec - tstart.tv_sec)* 1000000LL
      + (tstop.tv_usec - tstart.tv_usec);
      if (result.size() != buf.size())
        std::cout << "error content" << std::endl;
    }
    dp.stop(1);
    // We expect latency to be dp.meanInterval, but just to be sure also show
    // latency.
    std::cerr << "Average latency " << (latencySum / gLoopCount) << std::endl;
  }
  return 0;
}

int run_client_string(qi::Object* obj)
{
  qi::perf::DataPerfTimer dp ("Transport synchronous call");
  int rstart = 0;
  if (getenv("rstart")) rstart = strtol(getenv("rstart"), 0, 0);
  int rend = 20;
  if (getenv("rend")) rend = strtol(getenv("rend"), 0, 0);
  for (int i = rstart; i < rend; i+=2)
  {
    char character = 'c';
    unsigned int numBytes = (unsigned int)pow(2.0f, (int)i);
    std::string requeststr = std::string(numBytes, character);
    qi::Buffer buf;
    buf.reserve(numBytes);
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
  qi::DynamicObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  ob.advertiseMethod("replyBuf", &replyBuf);
  qi::Object obj(ob.object());

  session.connect("tcp://127.0.0.1:"+port);

  session.listen("tcp://0.0.0.0:0");
  session.registerService("serviceTest", obj);
  std::cout << "serviceTest ready." << std::endl;
  serverReady = true;
  while (!clientDone)
    qi::os::sleep(60);
  return 0;
}

int main(int argc, char **argv)
{
  qi::Application app(argc, argv);

  po::options_description desc(std::string("Usage:\n ")+argv[0]+"\nOptions");
  desc.add_options()
    ("help", "Print this help.")
    ("all", "(default) Run all in the same process.")
    ("client", "Run as a client.")
    ("server", "Run as a server.")
    ("gateway", "Run as a gateway.")
    ("local", "Run in local.")
    ("host", po::value<std::string>(), "Adresse of the host.")
    ("port", po::value<std::string>(), "Port of the server.")
    ("thread", po::value<int>()->default_value(1, "1"),
     "Number of thread to launch for clients");


  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
            .options(desc).run(), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }
  if (vm.count("client") + vm.count("server") + vm.count("all") + vm.count("gateway") + vm.count("local") > 1) {
    std::cerr << desc << std::endl << "You must put at most one option between [all|client|server|gateway|local]" << std::endl;
    return EXIT_FAILURE;
  }
  if (vm.count("host") > 1 || vm.count("port") > 1) {
    std::cerr << desc << std::endl << "[host] and [port] must at most appear one time." << std::endl;
    return EXIT_FAILURE;
  }

  std::string host, port;

  if (vm.count("host"))
    host = vm["host"].as<std::string>();

  if (vm.count("port"))
    port = vm["port"].as<std::string>();

  if (vm.count("client"))
  {
    int threadc = vm["thread"].as<int>();
    start_client(threadc, host, port);
  }
  else if (vm.count("server"))
  {
    return main_server(host, port);
  }
  else if (vm.count("gateway"))
  {
    return main_gateway(host, port);
  }
  else if (vm.count("local"))
  {
    return main_local();
  }
  else
  {
    //start the server
    allInOne = true;
    boost::thread threadServer1(boost::bind(&main_server, host, port));
    do {
      qi::os::msleep(500); // give it time to listen
    } while (!serverReady); // be nice for valgrind
    boost::thread threadServer2(boost::bind(&main_gateway, host, port));
    qi::os::sleep(1);
    std::string port = "12345";
    if (getenv("NO_GATEWAY"))
      port = sdPort;
    start_client(gThreadCount, host, port);
  }
  return 0;
}
