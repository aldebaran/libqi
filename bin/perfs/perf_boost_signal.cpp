/*
** Author(s):
**  - Nicolas CORNU <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <boost/program_options.hpp>
#include <boost/signal.hpp>
#include <boost/bind.hpp>

namespace po = boost::program_options;

#include <qi/application.hpp>
#include <qi/atomic.hpp>
#include <qitype/signal.hpp>
#include <qiperf/dataperfsuite.hpp>

qi::Atomic<int> glob(0);

// It's not atomic!!
static inline void resetAtomic(qi::Atomic<int>& val)
{
  int value = *val;
  for (int i = 0; i < value; ++i)
    --val;
}

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

  qi::DataPerfSuite out("boost", "signal", qi::DataPerfSuite::OutputData_Period, vm["output"].as<std::string>());

  boost::signal<void (void)> signal_1;

  signal_1.connect(boost::bind(&foo));

  qi::DataPerf dp;

  // Test signals without any arguments
  dp.start("Signal_void", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    signal_1();
    while (*glob != 1);
    --glob;
  }
  dp.stop();
  out << dp;

  boost::signal<void (int)> signal_2;

  signal_2.connect(boost::bind(&fooInt, _1));

  // Test signals with an int
  dp.start("Signal_int", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    signal_2(1);
    while (*glob != 1);
    --glob;
  }
  dp.stop();
  out << dp;

  // Test signals with a string of 32768 bytes
  boost::signal<void (std::string)> signal_3;

  signal_3.connect(boost::bind(&fooStr, _1));

  std::string s;
  for (unsigned int i = 0; i < 65535; ++i)
    s += "a";
  dp.start("Signal_Big_String", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    signal_3(s);
    while (*glob != 1);
    --glob;
  }
  dp.stop();
  out << dp;

  // Test signal with one int and 10 clients
  for (unsigned int i = 1; i < 10; ++i) {
    signal_2.connect(boost::bind(&fooInt, _1));
  }
  dp.start("Signal_Int_10clients", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    signal_2(1);

    while (*glob != 10);
    resetAtomic(glob);
  }
  dp.stop();
  out << dp;

  // Test signal with one string and 10 clients
  for (unsigned int i = 1; i < 10; ++i) {
    signal_3.connect(boost::bind(&fooStr, _1));
  }
  dp.start("Signal_Big_String_10clients", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    signal_3(s);

    while (*glob != 10);
    resetAtomic(glob);
  }
  dp.stop();
  out << dp;

  // Test signal with 10 args
  boost::signal<void (int, int, int, int, int, int)> signal_4;

  signal_4.connect(boost::bind(&fooSevenArgs, _1, _2, _3, _4, _5, _6));

  dp.start("Signal_7_int", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    signal_4(1, 1, 1, 1, 1, 1);

    while (*glob != 1);
    --glob;
  }
  dp.stop();
  out << dp;

  // Test signal with 10 args and 10 clients
  for (unsigned int i = 1; i < 10; ++i) {
    signal_4.connect(boost::bind(&fooSevenArgs, _1, _2, _3, _4, _5, _6));
  }

  dp.start("Signal_7_int_10_clients", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    signal_4(1, 1, 1, 1, 1, 1);

    while (*glob != 10);
    resetAtomic(glob);
  }
  dp.stop();
  out << dp;

  return EXIT_SUCCESS;
}
