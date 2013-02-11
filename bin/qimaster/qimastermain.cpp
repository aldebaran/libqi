/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <vector>
#include <map>

#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qi/application.hpp>

#include <qimessaging/servicedirectory.hpp>

#include <boost/program_options.hpp>

namespace po = boost::program_options;


int main(int argc, char *argv[])
{
  qi::Application app(argc, argv);

  // declare the program options
  po::options_description desc("Usage:\n  qi-master masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("master-address",
       po::value<std::string>()->default_value(std::string("tcp://0.0.0.0:5555")),
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
      std::cout << desc << std::endl;
      return 0;
    }

    if (vm.count("master-address") == 1)
    {
      std::string masterAddress;
      try {
        masterAddress = vm["master-address"].as<std::string>();
      } catch (const std::exception &) {
        return 1;
      }

      qi::Url sdUrl(masterAddress);
      qi::ServiceDirectory sd;

      if (sdUrl.protocol() == "tcps")
      {
        sd.setIdentity("tests/server.key", "tests/server.crt");
      }

      qi::Future<void> f = sd.listen(masterAddress);
      f.wait(3000);
      if (f.hasError())
      {
        qiLogError("qi-master") << "Failed to listen on " << masterAddress <<
          ". Is there another service running on this address?";
        exit(1);
      }

      qiLogInfo("qi-master") << "qi-master is listening on " << masterAddress;

      app.run();
    }
    else
    {
      std::cout << desc << std::endl;
    }
  }
  catch (const boost::program_options::error&)
  {
    std::cout << desc << std::endl;
  }

  return 0;
}
