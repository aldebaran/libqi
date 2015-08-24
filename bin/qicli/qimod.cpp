/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <iostream>
#include <iomanip>

#include <boost/program_options.hpp>
#include <qi/applicationsession.hpp>
#include <qi/anymodule.hpp>
#include <qi/path.hpp>
#include <qi/log.hpp>
#include <qi/iocolor.hpp>

qiLogCategory("qicli.qimod");

namespace po = boost::program_options;
using namespace qi;

void showModule(const AnyModule& module) {
  ModuleInfo mod = module.moduleInfo();
  std::cout << "[" << qi::StreamColor_Red << mod.name << qi::StreamColor_Reset << "]"
            << std::endl;

  std::cout << qi::StreamColor_Green << "  * " << qi::StreamColor_Fuchsia << "Info" << qi::StreamColor_Reset << ":"
            << std::endl;

  std::cout << "   path   " << mod.path << std::endl
            << "   type   " << mod.type << std::endl;

  qi::detail::printMetaObject(std::cout, module.metaObject());
}

void listMods() {
  std::vector<ModuleInfo> mods = qi::listModules();

  for (unsigned i = 0; i < mods.size(); ++i) {
    ModuleInfo mod = mods.at(i);

    std::cout << "[" << qi::StreamColor_Red << mod.name << qi::StreamColor_Reset << "]"
              << std::endl;
  }
}

int subCmd_mod(int argc, char** argv, qi::ApplicationSession& app)
{
  po::options_description desc("Usage:\n  qicli mod <module>");
  desc.add_options()
      ("help,h", "Print this help.")
      ("module,m", po::value<std::string>(), "module to inspect")
      ("list,l", "list all available modules")
      ;
  po::positional_options_description podesc;
  podesc.add("module", 1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).
            options(desc).positional(podesc).run(), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cout << desc << std::endl;
    return 0;
  }

  if (vm.count("list")) {
    listMods();
    return 0;
  }

  if (vm.count("module") == 0) {
    std::cout << desc << std::endl;
    std::cout << "error: please specify at least a module" << std::endl;
    return 1;
  }
  std::string pkgname = vm["module"].as<std::string>();

  AnyModule mod;
  try
  {
    mod = import(pkgname);
  }
  catch (const std::exception &e)
  {
    std::cerr << e.what() << std::endl;
    return -1;
  }

  showModule(mod);
  return 0;
}
