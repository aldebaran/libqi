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
#include <qimessaging/qt/qisession.h>
#include <qimessaging/qt/qiserver.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qurl.h>
#include "helloservice.h"
//#include "qiservicetest.hpp"

std::string reply(const std::string &msg) {
  std::cout << "Message recv:" << msg << std::endl;
  return msg + "bim";
}

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
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
      QString masterAddress = QString::fromUtf8(vm["master-address"].as<std::string>().c_str());
      QiSession     session;
      QHelloService hello;

      session.connect(masterAddress);
      session.waitForConnected();

#if 0
      srv.registerService("serviceTest", &hello);
      srv.listen(&session, QString::fromAscii("tcp://0.0.0.0:0"));
#endif

      std::cout << "ready." << std::endl;


      app.exec();

#if 0
      srv.close();
#endif
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
