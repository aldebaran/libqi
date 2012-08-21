#include <boost/program_options.hpp>

#include <qi/application.hpp>
#include <qimessaging/session.hpp>

static std::vector<std::string> modules;
static std::string listenTo;
static std::string sd;
QI_COMMAND_LINE_OPTIONS(
  ("module,m",value<std::vector<std::string> >(&modules) , "Load given module")
  ("listen,l", value<std::string>(&listenTo), "Address to listen to (default: localhost)")
  ("master-address,m", value<std::string>(&sd), "Address of the service directory")
  )


int main(int argc, char** argv)
{
  qi::Application app(argc, argv);
  qi::Session s;
  qiLogDebug("qi.launch") << "Connection to sd at " << sd;
  s.connect(sd);
  if (listenTo.empty())
    listenTo = "tcp://localhost:0";
  s.listen(listenTo);
  for (unsigned i=0; i<modules.size(); ++i)
  {
    qiLogDebug("qi.launch") << "loading " << modules[i];
    s.loadService(modules[i]);
  }
  app.run();
}
