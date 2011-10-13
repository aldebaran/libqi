/*
** Author(s):
**  - Nicolas HUREAU <nhureau@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#include <iostream>
#include <boost/program_options.hpp>
#include <sys/select.h>

#include "server.hpp"

namespace po = boost::program_options;

static
void serve(const char* host, unsigned short port)
{
  qi::gateway::Server server(host, port);

  server.run();
}

int main(int argc, char *argv[])
{
  // declare the program options
  po::options_description desc(("Usage:\n  " + std::string(argv[0]) +
                                " [options]\nOptions").c_str());
  desc.add_options()
    ("help,h", "Print this help.")
    ("listen,l",
      po::value<std::string>()->default_value(std::string("127.0.0.1")),
      "Address to listen to")
    ("port,p",
      po::value<unsigned short>()->default_value(4242), "Port to listen on")
    ("daemon,d", "Daemonize the server");

  // allow listen address to be specified as the first arg
  po::positional_options_description pos;
  pos.add("listen", 1);

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

    if (vm.count("listen") && vm.count("port"))
    {
      serve(vm["listen"].as<std::string>().c_str(),
          vm["port"].as<unsigned short>());
    }
    else
      std::cerr << desc << "\n";
  }
  catch (const boost::program_options::error&)
  {
    std::cerr << desc << "\n";
  }

  return 0;
}
