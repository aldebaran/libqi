#include "qicli.hpp"
#include "grid.hpp"

int subCmd_top(int argc, char **argv, const MainOptions &options)
{
  Grid grid;

  grid.column("method's name")
      .column("call count")
      .column("cumulated call time")
      .column("min call time")
      .column("max call time");

  SessionHelper session(options.address, options.verbose);
  std::vector<qi::ServiceInfo> servs = session.services();
  while (true)
  {
    for (unsigned int i = 0; i < servs.size(); ++i)
    {
      qi::FutureSync<qi::ObjectPtr> futService = session.service(servs[i].name());
      if (futService.hasError())
      {
        std::cout << "\terror : " << futService.error() << std::endl;
        return 3;
      }
      qi::ObjectPtr obj = futService.value();
      qi::FutureSync<qi::ObjectStatistics> futStats = obj->call<qi::ObjectStatistics>("stats");
      if (futStats.hasError())
      {
        std::cout << "\terror : " << futStats.error() << std::endl;
        return 3;
      }
      qi::ObjectStatistics stats = futStats.value();
      qi::ObjectStatistics::const_iterator begin = stats.begin();
      qi::ObjectStatistics::const_iterator end = stats.end();

      for (; begin != end; ++begin)
      {
        qi::MetaMethod const *metaMethod = obj->metaObject().method((*begin).first);
        if (!metaMethod)
          continue;
        grid["method's name"].entry(boost::any(metaMethod->name()));
        grid["call count"].entry(boost::any((*begin).second.count));
        grid["cumulated call time"].entry(boost::any((*begin).second.cumulatedTime));
        grid["min call time"].entry(boost::any((*begin).second.minTime));
        grid["max call time"].entry(boost::any((*begin).second.maxTime));
      }

    }
    grid.display();
    grid.clear();
    qi::os::msleep(1000);
    system("clear");
  }
  return 0;
}
