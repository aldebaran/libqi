/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include "master.hpp"

#include <qimessaging/master.hpp>
#include <qi/os.hpp>

namespace qi {
namespace gateway {

Master::Master(const std::string& address)
  : address_(address)
{}

Master::~Master()
{}

void Master::run()
{
  qi::Master master(address_);
  master.run();

  if (master.isInitialized()) {
    while (1)
      qi::os::sleep(1);
  }
}

void Master::launch(const std::string& address)
{
  Master master(address);
  master.run();
}

} // namespace gateway
} // namespace qi
