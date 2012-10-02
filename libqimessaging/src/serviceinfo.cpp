/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <iostream>
#include <vector>
#include <map>

#include <qimessaging/serviceinfo.hpp>
#include <qimessaging/datastream.hpp>

namespace qi
{

  ServiceInfo::ServiceInfo()
    : _name(),
      _serviceId(),
      _machineId(),
      _processId(),
      _endpoints(),
      _reserved(0)
  {
  }

}; // !qi
