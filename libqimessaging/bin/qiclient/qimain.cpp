/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <map>

#include <qi/os.hpp>
#include <qi/application.hpp>
#include <qimessaging/session.hpp>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

void call(const std::string &addr)
{
  qi::Session session;
  session.connect(addr);

#if 0
  std::vector<qi::ServiceInfo> servs = session.services();
  std::cout << "available services:" << std::endl;
  for (unsigned int i = 0; i < servs.size(); ++i)
    std::cout << "* " << servs[i].name() << std::endl;
  std::cout << std::endl;
#endif

  qi::Future<qi::ObjectPtr> fut = session.service("serviceTest");
  fut.wait();
  if (fut.hasError())
  {
      std::cerr << "Error returned:" << fut.error() << std::endl;
      return;
  }

  qi::ObjectPtr obj = fut.value();
#if 0
  int i = 0;
  while (true) {
    std::string result = obj->call<std::string>("reply", "plaf");
    if (!( i % 1000))
      std::cout << "answer(" << i << "):" << result << std::endl;
    ++i;
  }
#endif
  std::string result = obj->call<std::string>("reply", "plaf");
  std::cout << "result:" << result << std::endl;

  session.close();
}


int main(int argc, char *argv[])
{
  qi::Application a(argc, argv);

  // declare the program options
  po::options_description desc("Usage:\n  qi masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("master-address",
       po::value<std::string>()->default_value(std::string("tcp://127.0.0.1:5555")),
       "The master address");

  // allow master address to be specified as the first arg
  po::positional_options_description pos;
  pos.add("master-address", 1);

  // parse and store
  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(argc, argv).
              options(desc).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
      std::cout << desc << "\n";
      return 0;
    }

    if (vm.count("master-address") == 1)
    {
      std::string masteraddr = vm["master-address"].as<std::string>();
      call(masteraddr);
    }
    else
    {
      std::cout << desc << "\n";
    }
  } catch (const boost::program_options::error&) {
    std::cout << desc << "\n";
  }

  return 0;
}
