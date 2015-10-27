/*
** Author(s):
**  - Herve Cuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <string>

#ifdef WITH_BREAKPAD
# include <breakpad/breakpad.h>
#endif

#include <boost/program_options.hpp>
#include <qi/os.hpp>
#include <qi/application.hpp>
#include <qi/log.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/url.hpp>

namespace po = boost::program_options;

qiLogCategory("qi.gateway.main");

static const unsigned int NB_RETRY_RECONNECT = 5;
static const qi::Duration RETRY_RECONNECT_WAIT = qi::Seconds(2);

static bool attachToServiceDirectory(qi::Gateway& gw, const qi::Url& url)
{
  qi::Future<void> fut = gw.attachToServiceDirectory(url);

  return !fut.hasError();
}

int main(int argc, char *argv[])
{
  qi::Application app(argc, argv);

#if defined(WITH_BREAKPAD) && defined(GATEWAY_BUILD_TAG)
  BreakpadExceptionHandler eh(BREAKPAD_DUMP_DIR);
  eh.setBuildTag(GATEWAY_BUILD_TAG);
  qiLogInfo() << "Build tag: " GATEWAY_BUILD_TAG;
#endif


  // declare the program options
  po::options_description desc("Usage:\n  qi-service masterAddress [options]\nOptions");
  desc.add_options()
      ("help", "Print this help.")
      ("qi-url",
       po::value<std::string>()->default_value(std::string("tcp://127.0.0.1:9559")),
       "The master address")
      ("qi-listen-url",
       po::value<std::string>()->default_value(std::string("tcp://0.0.0.0:9559")),
       "The gateway address")
      ("gateway-type",
       po::value<std::string>()->default_value(std::string("local")),
       "The gateway type");

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
      return EXIT_SUCCESS;
    }

    const std::string gatewayAddress = vm["qi-listen-url"].as<std::string>();
    const std::string masterAddress = vm["qi-url"].as<std::string>();
    const std::string gatewayType = vm["gateway-type"].as<std::string>();

    if (gatewayType == "local")
    {
      qi::Gateway gateway;
      bool ok = false;
      for (unsigned i = 0; i < NB_RETRY_RECONNECT && !(ok = attachToServiceDirectory(gateway, qi::Url(masterAddress))); ++i)
      {
        qiLogInfo() << "Connection to SD failed, retrying in 2 seconds";
        qi::sleepFor(RETRY_RECONNECT_WAIT);
      }
      if (!ok || !gateway.listen(gatewayAddress))
      {
        qiLogError() << "Failed to launch gateway.";
        return EXIT_FAILURE;
      }
      qiLogInfo() << "Local gateway ready: " << gatewayAddress << std::endl;
      app.run();
    }
    else if (gatewayType == "remote")
    {
      qiLogError() << "Not implemented";
      return EXIT_FAILURE;
    }
    else if (gatewayType == "reverse")
    {
      qiLogError() << "Not implemented";
      return EXIT_FAILURE;
    }
    else
    {
      qiLogError() << "unknown type: " << gatewayType;
      return EXIT_FAILURE;
    }
  }
  catch (const boost::program_options::error&)
  {
    qiLogError() << desc;
  }

  return EXIT_SUCCESS;
}
