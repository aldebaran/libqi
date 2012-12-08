/*
 * Copyright (c) 2012 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

/*
 * This is a simple QiPerf example
 */

#include <iostream>
#include <qiperf/dataperfsuite.hpp>
#include <qi/os.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char *argv[])
{
  po::options_description desc;
  desc.add_options()
    ("help,h", "Print this help.");

  desc.add(qi::details::getPerfOptions());

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  qi::DataPerfSuite out("qiperf", "example_qiperf", qi::DataPerfSuite::OutputData_Period, vm["output"].as<std::string>());

  const unsigned count = 10;

  qi::DataPerf dp;
  dp.start(std::string("My_Stupid_Bench"), count);

  for (unsigned int i = 0; i < count; ++i) {
    qi::os::msleep(500);
  }
  dp.stop();
  out << dp;
  out.close();

  return EXIT_SUCCESS;
}
