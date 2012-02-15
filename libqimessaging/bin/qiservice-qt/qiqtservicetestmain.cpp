/*
** Author(s):
**  - Laurent LEC        <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>

#include <boost/program_options.hpp>

#include <qi/os.hpp>
#include <qimessaging/qisession.h>
#include <qimessaging/qiserver.h>

//#include "qiservicetest.hpp"

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
       po::value<std::string>()->default_value(std::string("127.0.0.1:5555")),
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
      QiSession session;
      QObject obj;
      QiServer srv;
      //obj.advertiseMethod("reply", &reply);

      srv.advertiseService("serviceTest", &obj);
      srv.start("127.0.0.1", 9571, &session);

      std::cout << "ready." << std::endl;

      std::string masterAddress = vm["master-address"].as<std::string>();
      session.connect(QString::fromStdString(masterAddress));
      session.waitForConnected();
      session.registerEndPoint(QUrl::QUrl("tcp://127.0.0.1:9571/"));

      while (1)
        qi::os::sleep(1);

      session.unregisterEndPoint(QUrl::QUrl("tcp://127.0.0.1:9571/"));
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
