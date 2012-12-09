/*
** Author(s):
**  - Nicolas CORNU <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/


#include <iostream>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

#include <qi/application.hpp>
#include <qiperf/dataperfsuite.hpp>

#include "perf_qt_signal.hpp"


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

  qi::DataPerfSuite out("qt", "signal_direct", qi::DataPerfSuite::OutputData_Period, vm["output"].as<std::string>());


  qi::DataPerf dp;

  Foo foo;
  QObject::connect(&foo, SIGNAL(signal_1()), &foo, SLOT(foo()), Qt::DirectConnection);
  // Test signals without any arguments
  dp.start("Signal_void", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    foo.emitSignal_1();
    while (foo.glob != 1);
    foo.glob = 0;
  }
  dp.stop();
  out << dp;


  QObject::connect(&foo, SIGNAL(signal_2(int)), &foo, SLOT(fooInt(int)), Qt::DirectConnection);

  // Test signals with an int
  dp.start("Signal_int", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    foo.emitSignal_2(42);
    while (foo.glob != 1);
    foo.glob = 0;
  }
  dp.stop();
  out << dp;

  // Test signals with a string of 32768 bytes
  QObject::connect(&foo, SIGNAL(signal_3(std::string)), &foo, SLOT(fooStr(std::string)), Qt::DirectConnection);

  std::string s;
  for (unsigned int i = 0; i < 65535; ++i)
    s += "a";
  dp.start("Signal_Big_String", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    foo.emitSignal_3(s);
    while (foo.glob != 1);
    foo.glob = 0;
  }
  dp.stop();
  out << dp;

  // Test signal with one int and 10 clients
  for (unsigned int i = 1; i < 10; ++i) {
    QObject::connect(&foo, SIGNAL(signal_2(int)), &foo, SLOT(fooInt(int)), Qt::DirectConnection);
  }
  dp.start("Signal_Int_10clients", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    foo.emitSignal_2(1);

    while (foo.glob != 10);
    foo.glob = 0;
  }
  dp.stop();
  out << dp;

  // Test signal with one string and 10 clients
  for (unsigned int i = 1; i < 10; ++i) {
    QObject::connect(&foo, SIGNAL(signal_3(std::string)), &foo, SLOT(fooStr(std::string)), Qt::DirectConnection);
  }
  dp.start("Signal_Big_String_10clients", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    foo.emitSignal_3(s);

    while (foo.glob != 10);
    foo.glob = 0;
  }
  dp.stop();
  out << dp;

  // Test signal with 7 args

  QObject::connect(&foo, SIGNAL(signal_4(int,int,int,int,int,int,int)), &foo, SLOT(fooSevenArgs(int,int,int,int,int,int,int)), Qt::DirectConnection);

  dp.start("Signal_7_int", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    foo.emitSignal_4(1, 1, 1, 1, 1, 1, 1);

    while (foo.glob != 1);
    foo.glob = 0;
  }
  dp.stop();
  out << dp;

  // Test signal with 7 args and 10 clients
  for (unsigned int i = 1; i < 10; ++i) {
    QObject::connect(&foo, SIGNAL(signal_4(int,int,int,int,int,int,int)), &foo, SLOT(fooSevenArgs(int,int,int,int,int,int,int)), Qt::DirectConnection);
  }

  dp.start("Signal_7_int_10_clients", 10000);
  for (unsigned int i = 0; i < 10000; ++i) {
    foo.emitSignal_4(1, 1, 1, 1, 1, 1, 1);

    while (foo.glob != 10);
    foo.glob = 0;
  }
  dp.stop();
  out << dp;

  return EXIT_SUCCESS;
}

