/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <iostream>
#include <string>
#include <map>
#include <qi/messaging.hpp>
#include <qi/perf/sleep.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

void qi_call(std::string addr) {
  qi::Context* context = new qi::Context();
  qi::Client client("qi command line client", context);
  client.connect(addr);
  if (!client.isInitialized()) {
    return;
  }
  //client.call("master.listServices::{ss}")

  typedef std::map<std::string, std::string>  StringMap;
  typedef std::map<std::string, StringMap>    MapMap;
  typedef std::vector<std::string>            VString;
  StringMap::const_iterator mit;
  VString::const_iterator   vit;

  StringMap serviceMap;
  VString   topics;
  VString   endpointsIDs;
  VString   machinesIDs;
  MapMap    machines;
  MapMap    endpoints;
  // DATA GATHERING ------------
  try {
    serviceMap = client.call< StringMap >("master.listServices");
    topics = client.call< VString >("master.listTopics");
    machinesIDs = client.call< VString >("master.listMachines");
    endpointsIDs = client.call< VString >("master.listEndpoints");
    
    for (vit = machinesIDs.begin(); vit != machinesIDs.end(); ++vit) {
      machines.insert(std::make_pair(*vit, client.call< StringMap >("master.getMachine", *vit)));
    }
    for (vit = endpointsIDs.begin(); vit != endpointsIDs.end(); ++vit) {
      endpoints.insert(std::make_pair(*vit, client.call< StringMap >("master.getEndpoint", *vit)));
    }
  } catch(const std::exception e) {
    std::cout << "Failed to gather data from master. Reason: " << e.what() << std::endl;
  }
  // ----------------------------

  std::cout << "Services:" << std::endl;
  for (mit = serviceMap.begin(); mit != serviceMap.end(); ++mit)
  {
    std::cout << mit->first << " :" << mit->second << std::endl;
  }

  std::cout << "Topics:" << std::endl;
  for (vit = topics.begin(); vit != topics.end(); ++vit)
  {
    std::cout << *vit << std::endl;
  }

  std::cout << "Machines:" << std::endl;
  for (vit = machinesIDs.begin(); vit != machinesIDs.end(); ++vit)
  {
    std::cout << *vit << std::endl;
  }

  std::cout << "Endpoints:" << std::endl;
  for (vit = endpointsIDs.begin(); vit != endpointsIDs.end(); ++vit)
  {
    std::cout << *vit << std::endl;
  }

  std::cout << "-----------" << std::endl;
  MapMap::const_iterator it;
  std::cout << "Endpoints:" << std::endl;
  for (it = endpoints.begin(); it != endpoints.end(); ++it)
  {
    const StringMap& m = it->second;
    for (mit = m.begin(); mit != m.end(); ++mit)
    {
      std::cout << mit->first << ": " << mit->second << std::endl;
    }
    std::cout << "--" << std::endl;
  }

  std::cout << "Machines:" << std::endl;
  for (it = machines.begin(); it != machines.end(); ++it)
  {
    const StringMap& m = it->second;
    for (mit = m.begin(); mit != m.end(); ++mit)
    {
      std::cout << mit->first << ": " << mit->second << std::endl;
    }
    std::cout << "--" << std::endl;
  }
  //  std::string locatethismethod("master.listServices::{ss}:");
  //  std::string result = client.call<std::string>("master.locateService", locatethismethod);
  //  std::cout << "result:" << result << std::endl;
}

int main(int argc, char *argv[])
{
  // declare the program options
  po::options_description desc("Usage:\n  qi masterAddress [options]\nOptions");
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
      std::string address = vm["master-address"].as<std::string>();
      qi_call(address);
    } else {
      std::cout << desc << "\n";
    }
  } catch (const boost::program_options::error&) {
    std::cout << desc << "\n";
  }

  return 0;
}
