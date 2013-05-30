/** Copyright (C) 2012 Aldebaran Robotics
*/

#include <boost/program_options.hpp>

#include <qi/log.hpp>
#include <qi/application.hpp>
#include <qimessaging/session.hpp>

qi::ObjectPtr o;
qi::MetaObject mo;
bool numeric = false;
bool printMo = false;
std::string sdUrl;
std::string objectName;
qiLogCategory("qitrace");
void cleanup()
{
  o->call<void>("enableTrace", false);
}

void onTrace(const qi::EventTrace& trace)
{
  static unsigned int maxLen = 0;
  std::string name = boost::lexical_cast<std::string>(trace.slotId);
  if (!numeric)
  {
    qi::MetaMethod* m = mo.method(trace.slotId);
    if (m)
      name = m->name();
    else
    {
      qi::MetaSignal* s = mo.signal(trace.slotId);
      if (s)
        name = s->name();
      else
        name = name + "(??" ")"; // trigraph protect mode on
    }
  }
  maxLen = std::max(maxLen, name.size());
  std::string spacing(maxLen + 8 - name.size(), ' ');
  std::cout << trace.id << '\t' << trace.kind << '\t' << name
   << spacing << trace.timestamp.tv_sec << '.' << trace.timestamp.tv_usec
   << "\t" << qi::encodeJSON(trace.arguments) << std::endl;
}

_QI_COMMAND_LINE_OPTIONS(
  "Qitrace options",
  ("numeric,n", bool_switch(&numeric), "Do not resolve slot Ids to names")
  ("service-directory,s", value<std::string>(&sdUrl), "url to connect to")
  ("object,o", value<std::string>(&objectName), "Object to monitor")
  ("print,p", bool_switch(&printMo), "Print out the Metaobject and exit")
  );

int main(int argc, char** argv)
{
  qi::Application app(argc, argv);
  if (objectName.empty())
  {
    std::cerr << "Object must be specified with -o" << std::endl;
    return 1;
  }
  qi::Session s;
  if (sdUrl.empty())
    sdUrl = "localhost";
  qi::Url url(sdUrl, "tcp", 9559);
  qiLogVerbose() << "Connecting to sd";
  s.connect(sdUrl);
  qiLogVerbose() << "Fetching service";
  qi::ObjectPtr o = s.service(objectName);
  if (!o)
  {
    std::cerr << "Service " << objectName << " not found" << std::endl;
    return 1;
  }
  mo = o->metaObject();
  if (printMo)
  {
    qi::details::printMetaObject(std::cout, mo);
    return 0;
  }
  qiLogVerbose() << "Connecting to trace signal";
  o->connect("traceObject", &onTrace);
  qiLogVerbose() << "Enabling trace";
  o->call<void>("enableTrace", true);
  app.atStop(&cleanup);
  app.run();
  return 0;
}
