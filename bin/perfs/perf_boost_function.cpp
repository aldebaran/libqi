/*
** Author(s):
**  - Nicolas CORNU <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <boost/function.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <qi/application.hpp>
#include <qi/atomic.hpp>
#include <qitype/signal.hpp>
#include <qiperf/dataperfsuite.hpp>

qi::Atomic<int> glob(0);

void foo()
{
  ++glob;
}

void fooInt(int a)
{
  ++glob;
}

void fooConstRStr(const std::string& a)
{
  ++glob;
}

void fooStr(std::string a)
{
  ++glob;
}

void fooSevenArgs(int a1, int a2, int a3, int a4, int a5, int a6)
{
  ++glob;
}

int main(int argc, char *argv[])
{
  qi::Application app(argc, argv);

  po::options_description desc(std::string("Usage:\n ")+argv[0]+"\n");
  desc.add_options()
    ("help,h", "Print this help.");

  desc.add(qi::details::getPerfOptions());

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv)
            .options(desc).allow_unregistered().run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  qi::DataPerfSuite out("boost", "function", qi::DataPerfSuite::OutputData_Period, vm["output"].as<std::string>());

  qi::DataPerf dp;

  boost::function<void(void)> f_1;
  f_1 = foo;

  // Test signals without any arguments
  dp.start("Signal_void", 10000000);
  for (unsigned int i = 0; i < 10000000; ++i) {
    f_1();
  }
  dp.stop();
  out << dp;

  boost::function<void (int)> f_2;
  f_2 = fooInt;

  // Test signals with an int
  dp.start("Signal_int", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    f_2(1);
  }
  dp.stop();
  out << dp;

  // Test signals with a string of 32768 bytes
  boost::function<void (std::string)> f_3;
  f_3 = fooStr;

  std::string s;
  for (unsigned int i = 0; i < 65535; ++i)
    s += "a";
  dp.start("Signal_Big_String", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    f_3(s);
  }
  dp.stop();
  out << dp;

  // Test signal with 10 args
  boost::function<void (int, int, int, int, int, int)> f_4;
  f_4 = fooSevenArgs;

  dp.start("Signal_7_int", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    f_4(1, 1, 1, 1, 1, 1);
  }
  dp.stop();
  out << dp;

  return EXIT_SUCCESS;
}
