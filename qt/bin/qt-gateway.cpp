/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qimessaging/qt/QiGateway>

#include <boost/program_options.hpp>
#include <QCoreApplication>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  // declare the program options
  po::options_description desc("Usage:\n  qi-service masterAddress [options]\nOptions");
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

    std::string gatewayAddress = vm["gateway-address"].as<std::string>();
    std::string masterAddress = vm["master-address"].as<std::string>();

    QiGateway gw;
    QUrl serviceDirectoryUrl(masterAddress.c_str());
    gw.attachToServiceDirectory(serviceDirectoryUrl);
    QUrl listenUrl(gatewayAddress.c_str());
    gw.listen(listenUrl);

    return a.exec();
  }
  catch (const boost::program_options::error&)
  {
    std::cout << desc << "\n";
  }

  return 0;
}
