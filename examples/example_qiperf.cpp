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

  qi::DataPerfSuite::OutputType type;
  std::string backend = vm["backend"].as<std::string>();
  if (backend == "normal")
    type = qi::DataPerfSuite::OutputType_Normal;
  else if (backend == "codespeed")
    type = qi::DataPerfSuite::OutputType_Codespeed;
  else {
    std::cerr << "This backend doesn't exist, fallback in [normal]." << std::endl;
    type = qi::DataPerfSuite::OutputType_Normal;
  }

  qi::DataPerfSuite* out = new qi::DataPerfSuite("qiperf", "example_qiperf", type, vm["output"].as<std::string>());

  qi::DataPerf dp;
  for (unsigned int i = 0; i < 10; ++i) {
    dp.start(std::string("My_Stupid_Bench"));
    qi::os::msleep(500);
    dp.stop();
    *out << dp;
  }
  delete out;

  return EXIT_SUCCESS;
}
