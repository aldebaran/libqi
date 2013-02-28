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

#include <qi/os.hpp>
#include <qi/application.hpp>
#include <qimessaging/url.hpp>
#include <qimessaging/transportsocket.hpp>
#include <qimessaging/session.hpp>
#include <qiperf/dataperfsuite.hpp>
#include <qiperf/dataperf.hpp>

#include <qimessaging/servicedirectory.hpp>
#include <qimessaging/gateway.hpp>
#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>

qiLogCategory("perf_transport");

static int gLoopCount = 10000;
static bool valgrind = false;
static const int gThreadCount = 1;
static bool allInOne = false; // True if sd/server/client are in this process
static qi::Url serverUrl;
static qi::Url gateUrl;
static bool clientDone = false;
static bool serverReady = false;
static qi::DataPerfSuite* out;
static bool noGateway = true;
static int rstart = 0;
static int rend = 20;
static int pipeline = 1;
static bool threadsafe = false;
static int msDelay = 0;

static int run_client(qi::ObjectPtr obj);

std::string reply(const std::string &msg)
{
  if (msDelay)
    qi::os::msleep(msDelay);
  return msg;
}

qi::Buffer replyBuf(const qi::Buffer& buf)
{
  qiLogDebug() << "enter";
  if (msDelay)
    qi::os::msleep(msDelay);
  qi::Buffer res = qi::Buffer(buf);
  qiLogDebug() << "leave";
  return res;
}

qi::ObjectPtr make_service()
{
  qi::GenericObjectBuilder ob;
  if (threadsafe)
    ob.setThreadingModel(qi::ObjectThreadingModel_MultiThread);
  ob.advertiseMethod("reply", &reply);
  ob.advertiseMethod("replyBuf", &replyBuf);
  qi::ObjectPtr obj(ob.object());
  return obj;
}

int main_local()
{
  qi::ObjectPtr obj = make_service();
  run_client(obj);
  return 0;
}

int main_client(std::string QI_UNUSED(src))
{
  if (valgrind)
    qi::os::msleep(3000);
  qi::Session session;
  qi::Url url = (allInOne && !noGateway)? gateUrl : serverUrl;
  std::cout <<"Connection to sd... "<< url.str() << std::endl;
  qi::FutureSync<void> isConnected = session.connect(url);
  isConnected.wait(3000);
  if (isConnected.hasError()) {
    std::cerr << "Can't connect to " << session.url().str() << std::endl;
    return -1;
  }
  std::cout <<"Getting service... " << std::endl;
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

  unsigned int numBytes = 1;
  std::vector<qi::Future<qi::Buffer> > ops;
  ops.resize(pipeline);
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
      qi::os::timeval tstart, tstop;
      qi::os::gettimeofday(&tstart);
      if (j >= pipeline)
      {
        ops[j % pipeline].wait();
        if (ops[j % pipeline].value().size() != buf.size())
          std::cout << "error content" << std::endl;
      }
      qi::Future<qi::Buffer> result = obj->call<qi::Buffer>("replyBuf", buf);
      ops[j % pipeline] = result;
      qi::os::gettimeofday(&tstop);
      latencySum += (tstop.tv_sec - tstart.tv_sec)* 1000000LL
        + (tstop.tv_usec - tstart.tv_usec);

    }
    for (int j=0; j<pipeline; ++j)
      ops[j].wait();

    dp.stop();
    *out << dp;

    numBytes <<= 2;

    // We expect latency to be dp.meanInterval, but just to be sure also show
    // latency.
    std::cerr << "Average latency " << (latencySum / gLoopCount) << std::endl;
  }
  return 0;
}

void start_client(int count)
{
  boost::thread thd[100];

  assert(count < 100);

  for (int i = 0; i < count; ++i)
  {
    std::stringstream ss;
    ss << "remote" << i;
    std::cout << "starting thread: " << ss.str() << std::endl;
    thd[i] = boost::thread(boost::bind(&main_client, ss.str()));
  }

  for (int i = 0; i < count; ++i)
    thd[i].join();
  clientDone = true;
}

#include <qi/os.hpp>

int main_gateway(const qi::Url& serverUrl)
{
  qi::Gateway       gate;

  gate.attachToServiceDirectory(serverUrl);

  gateUrl = "tcp://0.0.0.0:0";
  gate.listen(gateUrl);
  gateUrl = gate.endpoints()[0];

  std::cout << "Gateway listening on " << gateUrl.str() << std::endl;

  while (!clientDone)
    qi::os::sleep(60);

  return 0;
}

int main_server()
{
  qi::ServiceDirectory sd;
  if (sd.listen(serverUrl).hasError()) {
    std::cerr << "Service directory can't listen on " << serverUrl.str() << "." << std::endl;
    return 1;
  }
  serverUrl = sd.endpoints()[0];
  std::cout << "Service Directory ready on " << serverUrl.str() << std::endl;

  qi::Session       session;
  qi::ObjectPtr obj = make_service();

  session.connect(sd.endpoints()[0]);

  qi::Url sessionUrl("tcp://0.0.0.0:0");
  session.listen(sessionUrl);
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

  std::string usage = "If no mode is specified, run client and server in same process\n";

  po::options_description desc(std::string("Usage:\n ")+argv[0]+"\n" + usage);
  desc.add_options()
    ("help,h", "Print this help.")
    ("all", po::value<std::string>()->default_value("tcp://0.0.0.0:0"), "(default) Run all in the same process.")
    ("client,c", po::value<std::string>(), "Run as a client (tcp://xxx.xxx.xxx.xxx:xxxxx).")
    ("server,s", po::value<std::string>()->implicit_value("tcp://0.0.0.0:0"), "Run as a server.")
    ("gateway", po::value<std::string>(), "Run as a gateway.")
    ("local", "Run in local.")
    ("thread", po::value<int>()->default_value(1, "1"),
     "Number of thread to launch for clients")
    ("valgrind", "Set low loopcount and wait for valgrind.")
    ("gateway", "Run without gateway.")
    ("rstart", po::value<int>()->default_value(0, "0"), "rstart")
    ("rend", po::value<int>()->default_value(20, "20"), "rend")
    ("pipeline", po::value<int>()->default_value(1, "1"), "Max number of parallel calls to run")
    ("threadsafe", po::bool_switch()->default_value(false), "Declare threadsafe service")
    ("msdelay", po::value<int>()->default_value(0, "0"), "Delay in milliseconds to simulate long call")
    ;

  desc.add(qi::details::getPerfOptions());

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
            .options(desc).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  if (vm.count("gateway"))
    noGateway = false;

  if (vm.count("valgrind")) {
    gLoopCount = 500;
    valgrind = true;
  }


  if (vm.count("client") + vm.count("server") + !vm["all"].defaulted() + vm.count("gateway") + vm.count("local") > 1) {
    std::cerr << desc << std::endl << "You must put at most one option between [all|client|server|gateway|local]" << std::endl;
    return EXIT_FAILURE;
  }

  if (!vm["thread"].defaulted() && (vm.count("server") != 0 || vm.count("gateway") != 0)) {
    std::cerr << "[thread] is useless with [server] or [gateway]" << std::endl;
  }

  rstart = vm["rstart"].as<int>();
  rend   = vm["rend"].as<int>();
  pipeline = vm["pipeline"].as<int>();
  threadsafe = vm["threadsafe"].as<bool>();
  msDelay = vm["msdelay"].as<int>();

  out = new qi::DataPerfSuite("qimessaging", "transport", qi::DataPerfSuite::OutputData_Period, vm["output"].as<std::string>());

  if (vm.count("client"))
  {
    serverUrl = qi::Url(vm["client"].as<std::string>());
    int threadc = vm["thread"].as<int>();
    start_client(threadc);
  } else if (vm.count("server")) {
    serverUrl = vm["server"].as<std::string>();
    return main_server();
  } else if (vm.count("gateway")) {
    serverUrl = vm["gateway"].as<std::string>();
    return main_gateway(serverUrl);
  } else if (vm.count("local")) {
    return main_local();
  } else {
    //start the server
    int threadc = vm["thread"].as<int>();
    serverUrl = vm["all"].as<std::string>();
    allInOne = true;
    boost::thread threadServer1(boost::bind(&main_server));
    do {
      qi::os::msleep(500); // give it time to listen
    } while (!serverReady); // be nice for valgrind
    if (!noGateway)
      boost::thread threadServer2(boost::bind(&main_gateway, serverUrl));
    qi::os::sleep(1);

    start_client(threadc);
  }

  delete out;

  return EXIT_SUCCESS;
}
