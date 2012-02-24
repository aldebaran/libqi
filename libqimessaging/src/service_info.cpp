/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#include <iostream>
#include <vector>
#include <map>

#include <qimessaging/service_info.hpp>
#include <qimessaging/datastream.hpp>

namespace qi
{

  qi::DataStream &operator<<(qi::DataStream &stream, const ServiceInfo &sinfo) {
    stream << sinfo.name()
           << sinfo.serviceId()
           << sinfo.machineId()
           << sinfo.processId()
           << sinfo.endpoints();
    return stream;
  }

  qi::DataStream &operator>>(qi::DataStream &stream, ServiceInfo &sinfo) {
    stream >> sinfo._name;
    stream >> sinfo._serviceId;
    stream >> sinfo._machineId;
    stream >> sinfo._processId;
    stream >> sinfo._endpoints;
    return stream;
  }

}; // !qi
