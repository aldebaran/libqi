#include <boost/program_options.hpp>

#include <qimessaging/applicationsession.hpp>

qiLogCategory("qilaunch");

static std::vector<std::string> modules;
static bool any = false;

_QI_COMMAND_LINE_OPTIONS(
  "Launcher options",
  ("module,m",value<std::vector<std::string> >(&modules) , "Load given module")
  ("all,a", bool_switch(&any), "Listen on all interfaces (default localhost)")
)


int main(int argc, char** argv)
{
  /* User tool, be over-leniant in what we accept.
  */
  qi::ApplicationSession app(argc, argv);

  qiLogInfo() << "Connection to service directory at " << app.url().str();
  app.start();
  if (app.listenUrl().str().empty() && any)
  {
    qiLogInfo() << "Listening on tcp://0.0.0.0:0";
    app.session().listen("tcp://0.0.0.0:0");
  }
  for (int a = 1; a < argc; ++a)
  {
    if (argv[a][0] == '-')
      continue; // leftover arg, ignore
    modules.push_back(argv[a]);
  }
  for (unsigned i=0; i<modules.size(); ++i)
  {
    qiLogInfo() << "loading " << modules[i];
    app.session().loadService(modules[i]);
  }
  app.run();
}
