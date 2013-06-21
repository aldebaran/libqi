#include <map>
#include <string>
#include <iomanip>

#include <qimessaging/session.hpp>
#include <boost/date_time.hpp>

#include "qicli.hpp"

int readNumericInput()
{
  std::string c;
  std::cout << "Enter a service id:";
  std::getline(std::cin, c);
  return atoi(c.c_str());;
}

std::string readAlphaInput()
{
  std::string str;
  std::cout << "Enter a service name:";
  std::getline(std::cin, str);
  return str;
}

void showHelp(const po::options_description &desc)
{
  std::cout << std::left << desc << std::endl;
  std::cout << "sub commands:" << std::endl;
  std::cout << "  service [<ServicePattern>...]" << std::endl;
  std::cout << "  call    <ServicePattern.MethodPattern> [<JsonParameter>...]" << std::endl;
  std::cout << "  post    <ServicePattern.SignalPattern> [<JsonParameter>...]" << std::endl;
  std::cout << "  get     <ServicePattern.PropertyPattern>..." << std::endl;
  std::cout << "  set     <ServicePattern.PropertyPattern>... <JsonParameter>" << std::endl;
  std::cout << "  watch   <ServicePattern.SignalPattern>..." << std::endl;
}

bool poDefault(const po::command_line_parser &clp, po::variables_map &vm, const po::options_description &desc)
{
  try {
    po::store(const_cast<po::command_line_parser&>(clp).run(), vm);
    po::notify(vm);
  } catch (std::exception &e) {
    std::cout << desc;
    std::cerr << e.what() << std::endl;
    return false;
  }

  if (vm.count("help"))
  {
    std::cout << desc;
    exit(0);
  }
  return true;
}

std::string getTime()
{
  std::ostringstream msg;
  const boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
  const boost::posix_time::time_facet *f = new boost::posix_time::time_facet("%H:%M:%S");
  msg.imbue(std::locale(msg.getloc(),f));
  msg << now;
  return msg.str();
}

bool isNumber(const std::string &str)
{
  for (unsigned int i = 0; i < str.length(); ++i) {
    if (!::isdigit(str[i]))
      return false;
  }
  return true;
}
