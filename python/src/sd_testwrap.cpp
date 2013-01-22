/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <qi/os.hpp>
#include <qimessaging/servicedirectory.hpp>
#include "sd_testwrap.hpp"

servicedirectory::servicedirectory()
{
  qi::ServiceDirectory* sd = new qi::ServiceDirectory();

  sd->listen("tcp://127.0.0.1:0");

  _sd = sd;
}

char* servicedirectory::listen_url()
{
  qi::ServiceDirectory* sd = reinterpret_cast<qi::ServiceDirectory *>(_sd);

  if (!sd)
    return 0;

  return qi::os::strdup(sd->endpoints()[0].str().c_str());
}

void servicedirectory::close()
{
  qi::ServiceDirectory* sd = reinterpret_cast<qi::ServiceDirectory *>(_sd);

  sd->close();
}

servicedirectory::~servicedirectory()
{
  qi::ServiceDirectory* sd = reinterpret_cast<qi::ServiceDirectory *>(_sd);
  delete sd;
}
