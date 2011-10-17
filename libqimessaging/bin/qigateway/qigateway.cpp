/*
** Author(s):
**  - Nicolas HUREAU <nhureau@aldebaran-robotics.com>
**
** Copyright (C) 2011 Aldebaran Robotics
*/

#include <iostream>
#include <cstring>
#include <boost/program_options.hpp>
#include <qi/log.hpp>

#include <signal.h>
#include <ucontext.h>

#include "daemon.h"
#include "server.hpp"

namespace po = boost::program_options;

static
void sigint_handler(int signum, siginfo_t* info, void* vctx)
{
  ucontext_t* ctx = (ucontext_t*)vctx;

  qiLogInfo("qigateway", "Stopping QiMessaging Gateway");

  exit(0);
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
      po::value<unsigned short>()->default_value(12890), "Port to listen on")
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

    if (vm.count("daemon"))
      if (!daemonize())
        throw std::runtime_error("Could not daemonize");

    if (vm.count("listen") && vm.count("port"))
    {
      struct sigaction sigact;
      memset(&sigact, 0, sizeof (sigact));
      sigact.sa_sigaction = sigint_handler;

      if (sigaction(SIGINT, &sigact, NULL) == -1)
        throw std::runtime_error("Could not set SIG handler");

      qi::gateway::Server server(vm["listen"].as<std::string>().c_str(),
          vm["port"].as<unsigned short>());

      server.run();
    }
    else
    {
      std::cerr << desc << "\n";
      return 1;
    }
  }
  catch (const std::exception& e)
  {
    qiLogFatal("qigateway", "%s", e.what());
    return 1;
  }

  return 0;
}
