/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iostream>
#include <qimessaging/transport.hpp>
#include <qi/os.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class ServiceDirectoryServer : public qi::TransportServerDelegate
{
public:
  ServiceDirectoryServer()
  {
    ts = new qi::TransportServer();
    ts->setDelegate(this);
  }

  ~ServiceDirectoryServer()
  {
    delete ts;
  }

  void setThread(qi::NetworkThread *n)
  {
    nthd = n;
  }

  void start(const std::string &address, unsigned short port)
  {
    ts->start(address, port, nthd->getEventBase());
  }

  virtual void onConnected(const qi::Message &msg)
  {
    std::cout << "connected: " << msg.str() << std::endl;
  }

  virtual void onWrite(const qi::Message &msg)
  {
    std::cout << "written: " << msg.str() << std::endl;
  }

  virtual void onRead(const qi::Message &msg)
  {
    std::cout << msg << std::endl;

    if (msg.path() == "machines")
      machines(msg);
  }

  void machines(const qi::Message &msg) {
    qi::Message retval;
    retval.setType(qi::Message::Answer);
    retval.setId(msg.id());
    retval.setSource(msg.destination());
    retval.setDestination(msg.source());
    retval.setPath(msg.path());
    ts->send(retval);
  }

private:
  qi::NetworkThread   *nthd;
  qi::TransportServer *ts;
};


int main(int argc, char *argv[])
{
   // declare the program options
  po::options_description desc("Usage:\n  qi-master masterAddress [options]\nOptions");
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
  try {
    po::store(po::command_line_parser(argc, argv).
      options(desc).positional(pos).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << desc << "\n";
      return 0;
    }

    if(vm.count("master-address")==1) {
      std::string masterAddress = vm["master-address"].as<std::string>();
      //qi::Context* context = new qi::Context();
      //qi::Master master(masterAddress, context);

      // qi::Master master(masterAddress);
      // master.run();
      // if (master.isInitialized()) {
      //   while (1)
      //     sleep(1);
      // }
      qi::NetworkThread nt;
      ServiceDirectoryServer sds;

      sds.setThread(&nt);
      sleep(1);
      sds.start("127.0.0.1", 5555);
      std::cout << "ready." << std::endl;
      //sds.
      //qi::ServiceDirectory sd(masterAddress);
      //sd.exec();
      while (1)
        qi::os::sleep(1);
      //qi::Service svc(masterAddress);

      //svc.advertise("qi.servicedirectory",


    } else {
      std::cout << desc << "\n";
    }
  } catch (const boost::program_options::error&) {
    std::cout << desc << "\n";
  }

  return 0;
}
