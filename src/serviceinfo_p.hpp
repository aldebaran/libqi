#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_SERVICEINFO_P_HPP_
# define _QIMESSAGING_SERVICEINFO_P_HPP_

# include <string>

# include <qitype/type.hpp>
# include <qimessaging/serviceinfo.hpp>
# include <qimessaging/url.hpp>

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
    qi::UrlVector endpoints;
  };
}

#endif // _QIMESSAGING_SERVICEINFO_P_HPP_
