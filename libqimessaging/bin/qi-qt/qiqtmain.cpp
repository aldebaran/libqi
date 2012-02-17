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
#include <qimessaging/session.hpp>
#include <qimessaging/transport.hpp>
#include <qimessaging/object.hpp>

#include <boost/program_options.hpp>

#include <qimessaging/qisession.h>

namespace po = boost::program_options;

static int uniqueReqId = 200;

void call(const std::string &addr)
{
  QiSession session;
  session.connect(QString::fromStdString(addr));
  session.waitForConnected();

  QVector<QString> services = session.services();
  foreach (QString service, services)
    std::cout << "service named " << qPrintable(service) << std::endl;


  QObject *obj = session.service("serviceTest");
  QString ret;
  QMetaObject::invokeMethod(obj, "reply", Qt::DirectConnection,
                            Q_RETURN_ARG(QString, ret),
                            Q_ARG(QString, "plaf"));

  std::cout << "answer:" << ret.toStdString() << std::endl;
  qi::os::sleep(2);
}


int main(int argc, char *argv[])
{
  // declare the program options
  po::options_description desc("Usage:\n  qi masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("master-address",
       po::value<std::string>()->default_value(std::string("tcp://127.0.0.1:5555")),
       "The master address")
      ("gateway-address",
       po::value<std::string>()->default_value(std::string("tcp://127.0.0.1:12345")),
       "The gateway address");

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

    if (vm.count("master-address") == 1 &&
        vm.count("gateway-address") == 1)
    {
      std::string masteraddr = vm["master-address"].as<std::string>();
      call(masteraddr);

      std::string gatewayaddr = vm["gateway-address"].as<std::string>();
      call(gatewayaddr);
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
