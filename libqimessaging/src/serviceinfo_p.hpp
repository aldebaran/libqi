#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SERVICEINFO_P_HPP_
# define _QIMESSAGING_SERVICEINFO_P_HPP_

# include <string>
# include <vector>

# include <qitype/type.hpp>
# include <qimessaging/serviceinfo.hpp>

namespace qi
{
  class ServiceInfoPrivate
  {
  public:
    ServiceInfoPrivate();

    std::string name;
    unsigned int serviceId;
    std::string machineId;
    unsigned int processId;
    std::vector<std::string> endpoints;
  };
}

#endif // _QIMESSAGING_SERVICEINFO_P_HPP_
