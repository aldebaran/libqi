#include <iomanip>

#include <boost/program_options.hpp>
#include <qimessaging/session.hpp>

#include "qicli.hpp"

int subCmd_call(int argc, char **argv, const MainOptions &options)
{
  po::options_description     desc("Usage: qicli call Service.Method ARGS...");
  std::string                 fullName;
  std::vector<std::string>    argList;

  desc.add_options()
      ("method,m", po::value<std::string>(&fullName)->required(), "method's name")
      ("arg,a", po::value<std::vector<std::string> >(&argList), "method's args")
      ("help,h", "Print this help message and exit");

  po::positional_options_description positionalOptions;
  positionalOptions.add("method", 1);
  positionalOptions.add("arg", -1);

  po::variables_map vm;
  if (!poDefault(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions), vm, desc))
    return 1;
  SessionHelper session(options.address);
  std::string serviceName;
  std::string methodName;

  if (!::splitName(fullName, serviceName, methodName))
  {
    std::cerr << "error, Service.Method syntax not respected" << std::endl;
    return 1;
  }
  ServiceHelper service;

  if (!session.getServiceSync(serviceName, service))
    return 1;

  return service.call(methodName, argList);
}

int subCmd_post(int argc, char **argv, const MainOptions &options)
{
  po::options_description     desc("Usage: qicli post Service.Signal ARGS...");
  std::string                 fullName;
  std::vector<std::string>    argList;

  desc.add_options()
      ("signal,s", po::value<std::string>(&fullName)->required(), "signal's name")
      ("arg,a", po::value<std::vector<std::string> >(&argList), "method's args")
      ("help,h", "Print this help message and exit");

  po::positional_options_description positionalOptions;
  positionalOptions.add("signal", 1);
  positionalOptions.add("arg", -1);

  po::variables_map vm;
  if (!poDefault(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions), vm, desc))
    return 1;

  SessionHelper session(options.address);
  std::string serviceName;
  std::string signalName;

  if (!::splitName(fullName, serviceName, signalName))
  {
    std::cerr << "error, Service.Signal syntax not respected" << std::endl;
    return 1;
  }
  ServiceHelper service;

  if (!session.getServiceSync(serviceName, service))
    return 1;

  return service.post(signalName, argList);
}

int subCmd_service(int argc, char **argv, const MainOptions &options)
{
  po::options_description     desc("Usage: qicli service PATTERN...");
  std::vector<std::string>    serviceList;

  desc.add_options()
      ("service,s", po::value<std::vector<std::string> >(&serviceList), "service to display")
      ("help,h", "Print this help message and exit")
      ("list,l", "List services (default when no service specified)")
      ("details,z", "print services' details")
      ("all,a", "show hidden services, methods, signals and properties");

  po::positional_options_description positionalOptions;
  positionalOptions.add("service", -1);

  po::variables_map vm;
  if (!poDefault(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions), vm, desc))
    return 1;

  bool details = false;
  if (vm.count("details") && vm.count("list"))
    throw std::runtime_error("You cannot specify --list and --details together.");

  //smart details/list
  if (vm.count("details"))
    details = true;
  else if (vm.count("list"))
    details = false;
  else {
    //list/details not specified, use details if services have been specified.
    details = (serviceList.size()) != 0;
  }

  SessionHelper session(options.address);
  session.showServicesInfoPattern(serviceList, details);
  return 0;
}

int subCmd_watch(int argc, char **argv, const MainOptions &options)
{
  po::options_description desc("Usage: qicli watch Service.Signal");
  std::string             fullName;

  desc.add_options()
      ("signal,s", po::value<std::string>(&fullName)->required(), "signal's name")
      ("time,t", "Print time")
      ("help,h", "Print this help message and exit");

  po::positional_options_description positionalOptions;
  positionalOptions.add("signal", 1);

  po::variables_map vm;
  if (!poDefault(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions), vm, desc))
    return 1;

  SessionHelper session(options.address);
  std::string serviceName;
  std::string signalName;

  if (!::splitName(fullName, serviceName, signalName))
  {
    std::cerr << "error, Service.Signal syntax not respected" << std::endl;
    return 1;
  }
  ServiceHelper service;

  if (!session.getServiceSync(serviceName, service))
    return 1;

  return service.watchSignal(signalName, vm.count("time"));
}

int subCmd_get(int argc, char **argv, const MainOptions &options)
{
  po::options_description desc("Usage: qicli get Service.Propertie");
  std::string             fullName;

  desc.add_options()
      ("prop,p", po::value<std::string>(&fullName)->required(), "propertie's name")
      ("help,h", "Print this help message and exit");

  po::positional_options_description positionalOptions;
  positionalOptions.add("prop", 1);

  po::variables_map vm;
  if (!poDefault(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions), vm, desc))
    return 1;

  SessionHelper session(options.address);
  std::string serviceName;
  std::string propName;

  if (!::splitName(fullName, serviceName, propName))
  {
    std::cerr << "error, Service.Method syntax not respected" << std::endl;
    return 1;
  }
  ServiceHelper service;

  if (!session.getServiceSync(serviceName, service))
    return 1;

  return service.showProp(propName);
}

int subCmd_set(int argc, char **argv, const MainOptions &options)
{
  po::options_description desc("Usage: qicli get Service.Propertie");
  std::string             fullName;
  std::string             value;

  desc.add_options()
      ("prop,p", po::value<std::string>(&fullName)->required(), "propertie's name")
      ("value,v", po::value<std::string>(&value)->required(), "value to set")
      ("help,h", "Print this help message and exit");

  po::positional_options_description positionalOptions;
  positionalOptions.add("prop", 1);
  positionalOptions.add("value", 2);

  po::variables_map vm;
  if (!poDefault(po::command_line_parser(argc, argv).options(desc).positional(positionalOptions), vm, desc))
    return 1;

  SessionHelper session(options.address);
  std::string serviceName;
  std::string propName;

  if (!::splitName(fullName, serviceName, propName))
  {
    std::cerr << "error, Service.Method syntax not respected" << std::endl;
    return 1;
  }
  ServiceHelper service;

  if (!session.getServiceSync(serviceName, service))
    return 1;

  return service.setProp(propName, value);
}
