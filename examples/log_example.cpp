/*
 * Copyright (c) 2011, Aldebaran Robotics
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Aldebaran Robotics nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Aldebaran Robotics BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <qi/log.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

int main(int argc, char **argv)
{
  po::options_description desc("Allowed options");

  int globalVerbosity;

  // Used to parse options, as well as to put help message
  desc.add_options()
          ("help,h", "Produces help message")
          ("version", "Output NAOqi version.")
          ("verbose,v", "Set verbose verbosity.")
          ("debug,d", "Set debug verbosity.")
          ("quiet,q", "Do not show logs on console.")
          ("context,c", po::value<int>(), "Show context logs: [0-7] (0: none, 1: categories, 2: date, 3: file+line, 4: date+categories, 5: date+line+file, 6: categories+line+file, 7: all (date+categories+line+file+function)).")
          ("synchronous-log", "Activate synchronous logs.")
          ("log-level,L", po::value<int>(&globalVerbosity)->default_value(4), "Change the log minimum level: [0-6] (0: silent, 1: fatal, 2: error, 3: warning, 4: info, 5: verbose, 6: debug). Default: 4 (info)")
    ;

  // Map containing all the options with their values
  po::variables_map vm;

  // program option library throws all kind of errors, we just catch them
  // all, print usage and exit
  try
  {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
  }
  catch (po::error &e)
  {
    std::cerr << e.what() << std::endl;
    std::cout << desc << std::endl;
    exit(1);
  }

  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 0;
  }

  // set default log verbosity
  qi::log::setVerbosity(qi::log::info);
  // set default log context
  qi::log::setContext(0);

  if (vm.count("log-level"))
  {
    if (globalVerbosity > 0 && globalVerbosity <= 6)
      qi::log::setVerbosity((qi::log::LogLevel)globalVerbosity);
    if (globalVerbosity > 6)
      qi::log::setVerbosity(qi::log::debug);
    if (globalVerbosity <= 0)
      qi::log::setVerbosity(qi::log::silent);
  }

  // Remove consoleloghandler (default log handler)
  if (vm.count("quiet"))
    qi::log::removeLogHandler("consoleloghandler");

  if (vm.count("debug"))
    qi::log::setVerbosity(qi::log::debug);

  if (vm.count("verbose"))
    qi::log::setVerbosity(qi::log::verbose);

  if (vm.count("context"))
  {
    int globalContext = vm["context"].as<int>();

    if (globalContext < 0)
    {
      qi::log::setContext(0);
    }
    else if (globalContext > 7)
    {
      qi::log::setContext(7);
    }
    else
    {
      qi::log::setContext(globalContext);
    }
  }

  if (vm.count("synchronous-log"))
    qi::log::setSynchronousLog(true);

  qiLogFatal("core.log.example.1", "%d\n", 41);
  qiLogError("core.log.example.1", "%d\n", 42);
  qiLogWarning("core.log.example.1", "%d\n", 43);
  qiLogInfo("core.log.example.1", "%d\n", 44);
  qiLogVerbose("core.log.example.1", "%d\n", 45);
  qiLogDebug("core.log.example.1", "%d\n", 46);

  qiLogFatal("core.log.example.2")   << "f" << 4 << std::endl;
  qiLogError("core.log.example.2")   << "e" << 4 << std::endl;
  qiLogWarning("core.log.example.2") << "w" << 4 << std::endl;
  qiLogInfo("core.log.example.2")    << "i" << 4 << std::endl;
  qiLogVerbose("core.log.example.2") << "v" << 4 << std::endl;
  qiLogDebug("core.log.example.2")   << "d" << 4 << std::endl;

  qiLogFatal("core.log.example.1", "without '\\n': %d", 41);
  qiLogError("core.log.example.1", "without '\\n': %d", 42);
  qiLogWarning("core.log.example.1", "without '\\n': %d", 43);
  qiLogInfo("core.log.example.1", "without '\\n': %d", 44);
  qiLogVerbose("core.log.example.1", "without '\\n': %d", 45);
  qiLogDebug("core.log.example.1", "without '\\n': %d", 46);
  qiLogFatal("core.log.example.1", "");
  qiLogFatal("core.log.example.1", "\n");

  qiLogFatal("core.log.example.2")   << "f " << "without '\\n'";
  qiLogError("core.log.example.2")   << "e " << "without '\\n'";
  qiLogWarning("core.log.example.2") << "w " << "without '\\n'";
  qiLogInfo("core.log.example.2")    << "i " << "without '\\n'";
  qiLogVerbose("core.log.example.2") << "v " << "without '\\n'";
  qiLogDebug("core.log.example.2")   << "d " << "without '\\n'";
  qiLogFatal("core.log.example.2")   << "";
  qiLogFatal("core.log.example.2")   << std::endl;

  qiLogFatal("core.log.example.3", "%d", 21)   << "f" << 4 << std::endl;
  qiLogError("core.log.example.3", "%d", 21)   << "e" << 4 << std::endl;
  qiLogWarning("core.log.example.3", "%d", 21) << "w" << 4 << std::endl;
  qiLogInfo("core.log.example.3", "%d", 21)    << "i" << 4 << std::endl;
  qiLogVerbose("core.log.example.3", "%d", 21) << "v" << 4 << std::endl;
  qiLogDebug("core.log.example.3", "%d", 21)   << "d" << 4 << std::endl;

  //c style
  qiLogWarning("core.log.example.4",
               "Oups my buffer is too bad: %x\n",
               0x0BADCAFE);

  //c++ style
  qiLogError("core.log.example.4") << "Where is nao?"
                                   << " - Nao is in the kitchen."
                                   << " - How many are they? "
                                   << 42 << std::endl;

  //mixup style
  qiLogInfo("core.log.example.4", "%d %d ", 41, 42) << 43 << " " << 44
                                                    << std::endl;

  std::cout << "I've just finished to log!" << std::endl;
}
