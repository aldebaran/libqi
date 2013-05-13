/**
 * This is a simple QiMessaging example
 */

#include <qi/os.hpp>
#include <qimessaging/server.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

// Handler for the service you want to advertise
int getMeaningOfLife()
{
  return 42;
}

int main(int argc, char** argv)
{
  // declare the program options
  po::options_description desc(std::string("Usage:\n  ") + argv[0] +
                               " address [options]\nOptions");
  desc.add_options()
    ("help", "Print this help.")
    ("master-address",
    po::value<std::string>()->default_value(std::string("127.0.0.1:9559")),
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

    if(vm.count("master-address"))
    {
      // Create the server, giving it a name that helps to track it
      qi::Server server("deepThought");

      // Connect the server to the master
      server.connect(vm["master-address"].as<std::string>());

      // Advertise the service, giving it a name.
      server.advertiseService("deepThought.getMeaningOfLife", &getMeaningOfLife);

      // XXX: need a nice way to sleep
      while (true)
        qi::os::sleep(1);
    }
    else
      std::cout << desc << "\n";
  }
  catch (const boost::program_options::error&)
  {
    std::cout << desc << "\n";
  }

  return 0;
}
