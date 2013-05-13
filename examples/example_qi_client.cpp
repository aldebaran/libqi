/**
 * This is a simple QiMessaging example
 */

#include <iostream>
#include <qimessaging/client.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

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
      // Create a client
      qi::Client client("myClient");

      // Connect the client to the master
      client.connect(vm["master-address"].as<std::string>());

      // Call a method, expecting an int return type
      int theMeaningOfLife = client.call<int>("deepThought.getMeaningOfLife");
      std::cout << "Meaning of life:" << theMeaningOfLife << std::endl;
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
