/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Nicolas CORNU <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/


#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <qi/application.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qimessaging/session.hpp>
#include <qiperf/dataperfsuite.hpp>
#include <qiperf/dataperf.hpp>

#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/gateway.hpp>
#include <qimessaging/genericobject.hpp>
#include <qimessaging/genericobjectbuilder.hpp>

static int gLoopCount = getenv("VALGRIND")?500:10000;
static const int gThreadCount = 1;
static bool allInOne = false; // True if sd/server/client are in this process
static std::string serverPort;
static bool clientDone = false;
static bool serverReady = false;
int run_client(qi::ObjectPtr obj);
qi::DataPerfSuite* out;

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
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  ob.advertiseMethod("replyBuf", &replyBuf);
  qi::ObjectPtr obj(ob.object());
  if (getenv("SYNCHRONOUS"))
    obj->moveToEventLoop(0);
  run_client(obj);
  return 0;
}

int main_client(std::string QI_UNUSED(src), std::string addr)
{
  if (getenv("VALGRIND"))
    qi::os::msleep(3000);
  qi::Session session;
  std::cerr <<"Connection to sd... " << std::endl;
  session.connect(addr);
  std::cerr <<"Getting service... " << std::endl;
  qi::Future<qi::ObjectPtr> futobj = session.service("serviceTest");
  futobj.wait();
  qi::ObjectPtr obj = futobj.value();
  std::cerr <<"Done" << std::endl;
  if (!obj)
  {
    std::cerr << "cant get serviceTest " << futobj.error() << std::endl;
    return -1;
  }
  return run_client(obj);
}


int run_client(qi::ObjectPtr obj)
{
  qi::DataPerf dp;
  int rstart = 0;
  if (getenv("rstart")) rstart = strtol(getenv("rstart"), 0, 0);
  int rend = 20;
  if (getenv("rend")) rend = strtol(getenv("rend"), 0, 0);

  unsigned int numBytes = 1;
  for (int i = rstart; i < rend; i+=2)
  {
    std::ostringstream oss(std::ostringstream::out);
    oss << "transport_" << numBytes << "b";

    qi::Buffer buf;
    buf.reserve(numBytes);
    dp.start(oss.str(), gLoopCount, numBytes);
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

    dp.stop();
    *out << dp;

    numBytes <<= 2;

    // We expect latency to be dp.meanInterval, but just to be sure also show
    // latency.
    std::cerr << "Average latency " << (latencySum / gLoopCount) << std::endl;
  }
  return 0;
}

void start_client(int count, std::string addr)
{
  boost::thread thd[100];

  assert(count < 100);

  for (int i = 0; i < count; ++i)
  {
    std::stringstream ss;
    ss << "remote" << i;
    std::cout << "starting thread: " << ss.str() << std::endl;
    thd[i] = boost::thread(boost::bind(&main_client, ss.str(), addr));
  }

  for (int i = 0; i < count; ++i)
    thd[i].join();
  clientDone = true;
}

#include <qi/os.hpp>

int main_gateway(std::string addr)
{
  qi::Gateway       gate;

  gate.attachToServiceDirectory(addr);

  gate.listen("tcp://0.0.0.0:12345");

  std::cout << "Gateway listening on tcp://0.0.0.0:12345" << std::endl;

  while (!clientDone)
    qi::os::sleep(60);

  return 0;
}

int main_server(std::string addr)
{
  qi::ServiceDirectory sd;
  sd.listen(addr);
  serverPort = boost::lexical_cast<std::string>(sd.listenUrl().port());
  std::cout << "Service Directory ready on " << sd.listenUrl().str() << std::endl;

  qi::Session       session;
  qi::GenericObjectBuilder ob;
  ob.advertiseMethod("reply", &reply);
  ob.advertiseMethod("replyBuf", &replyBuf);
  qi::ObjectPtr obj(ob.object());

  session.connect("tcp://127.0.0.1:" + serverPort);

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
            .options(desc).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("client") + vm.count("server") + vm.count("all") + vm.count("gateway") + vm.count("local") > 1) {
    std::cerr << desc << std::endl << "You must put at most one option between [all|client|server|gateway|local]" << std::endl;
    return EXIT_FAILURE;
  }

  if (!vm["thread"].defaulted() && (vm.count("server") != 0 || vm.count("gateway") != 0)) {
    std::cerr << "[thread] is useless with [server] or [gateway]" << std::endl;
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

  out = new qi::DataPerfSuite("qimessaging", "transport", type, vm["output"].as<std::string>());

  if (vm.count("client"))
  {
    std::string addr = vm["client"].as<std::string>();
    int threadc = vm["thread"].as<int>();

    start_client(threadc, addr);
  } else if (vm.count("server")) {
    std::string addr = vm["server"].as<std::string>();
    return main_server(addr);
  } else if (vm.count("gateway")) {
    std::string addr = vm["gateway"].as<std::string>();
    return main_gateway(addr);
  } else if (vm.count("local")) {
    return main_local();
  } else {
    //start the server
    std::string addr = vm["all"].as<std::string>();
    allInOne = true;
    boost::thread threadServer1(boost::bind(&main_server, addr));
    do {
      qi::os::msleep(500); // give it time to listen
    } while (!serverReady); // be nice for valgrind
    if (!getenv("NO_GATEWAY"))
      boost::thread threadServer2(boost::bind(&main_gateway, "tcp://127.0.0.1:" + serverPort));
    qi::os::sleep(1);

    start_client(gThreadCount, "tcp://127.0.0.1:" + (getenv("NO_GATEWAY") ? serverPort : "12345"));
  }

  delete out;

  return EXIT_SUCCESS;
}
