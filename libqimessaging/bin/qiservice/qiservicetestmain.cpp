/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include <qi/os.hpp>
#include <qi/log.hpp>
#include <qimessaging/session.hpp>

std::string reply(const std::string &msg) {
  std::cout << "Message recv:" << msg << std::endl;
  return msg + "bim";
}

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  // declare the program options
  po::options_description desc("Usage:\n  qi-service masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("master-address",
       po::value<std::string>()->default_value(std::string("tcp://127.0.0.1:5555")),
       "The master address");

  // allow master address to be specified as the first arg
  po::positional_options_description pos;
  pos.add("master-address", 1);

  // Test hostIPAddrs
  std::map<std::string, std::vector<std::string> > ifsMap = qi::os::hostIPAddrs();

  if (ifsMap.empty())
   qiLogInfo("qimessaging.ServiceTest", "hostIPAddrs failed");

  if (ifsMap.empty() == false)
    for (std::map<std::string, std::vector<std::string> >::const_iterator adapter = ifsMap.begin();
         adapter != ifsMap.end();
         ++adapter)
    {
      for (std::vector<std::string>::const_iterator address = (*adapter).second.begin();
           address != (*adapter).second.end();
           ++address)
      {
        qiLogInfo("qimessaging.ServiceTest", "%s : %s", (*adapter).first.c_str(), (*address).c_str());
      }
    }

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
      std::string masterAddress = vm["master-address"].as<std::string>();
      qi::Session       session;
      qi::Object        obj;
      obj.advertiseMethod("reply", &reply);

      session.connect(masterAddress);
      session.waitForConnected();

      session.listen("tcp://0.0.0.0:0");
      unsigned int id = session.registerService("serviceTest", &obj);

#if 0
      // test unregistration
      session.unregisterService(id);
      id = session.registerService("serviceTest", &obj);
#endif

      if (id)
      {
        qiLogInfo("qimessaging.ServiceTest") << "registered as service #" << id << std::endl;
      }
      else
      {
        qiLogError("qimessaging.ServiceTest") << "registration failed..." << std::endl;
        exit(1);
      }
      session.join();

      session.unregisterService(id);
      session.close();
      session.disconnect();
    }
    else
    {
      std::cout << desc << "\n";
    }
  }
  catch (const boost::program_options::error&)
  {
    std::cout << desc << "\n";
  }

  return 0;
}
