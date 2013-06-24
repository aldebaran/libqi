#include <boost/program_options.hpp>

#include <qi/application.hpp>
#include <qimessaging/session.hpp>

qiLogCategory("qilaunch");

static std::vector<std::string> modules;
static std::string listenTo;
static std::string sd;
static std::string proto;
static bool any = false;

_QI_COMMAND_LINE_OPTIONS(
  "Launcher options",
  ("module,m",value<std::vector<std::string> >(&modules) , "Load given module")
  ("listen,l", value<std::string>(&listenTo), "Address to listen to (default: localhost)")
  ("protocol,p", value<std::string>(&proto), "Protocol to listen on (default: tcp)")
  ("all,a", bool_switch(&any), "Listen on all interfaces (default localhost)")
  ("service-directory,s", value<std::string>(&sd), "Address of the service directory (default: tcp://localhost:9559)")
)


int main(int argc, char** argv)
{
  /* User tool, be over-leniant in what we accept.
  */
  qi::Application app(argc, argv);
  qi::Session s;
  if (sd.empty())
    sd = "tcp://127.0.0.1:9559";
  else
  {
    size_t p = sd.find(':');
    if (p == sd.npos)
    { // no proto no port3
      sd = "tcp://" + sd + ":9559";
    }
    else
    {
      if (sd[p+1] != '/')
      { // not a proto: it is a port, so there is no proto
        sd = "tcp://" + sd;
      }
      // if user specified a proto, do not try to append a port
    }
  }
  qiLogInfo() << "Connection to sd at " << sd;
  s.connect(sd);
  if (listenTo.empty())
  {
    if (!proto.empty())
      listenTo = proto;
    else
      listenTo = "tcp";
    listenTo += "://";
    if (any)
      listenTo += "0.0.0.0";
    else
      listenTo += "localhost";
    listenTo += ":0";
  }
  qiLogInfo() << "Listening on " << listenTo;
  s.listen(listenTo);
  for (int a = 1; a < argc; ++a)
  {
    if (argv[a][0] == '-')
      continue; // leftover arg, ignore
    modules.push_back(argv[a]);
  }
  for (unsigned i=0; i<modules.size(); ++i)
  {
    qiLogInfo() << "loading " << modules[i];
    s.loadService(modules[i]);
  }
  app.run();
}
