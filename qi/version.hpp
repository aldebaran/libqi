#pragma once
/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_VERSION_HPP_
#define _QI_VERSION_HPP_

# include <qi/api.hpp>
# include <vector>
# include <string>

namespace qi {
  namespace version {

    //convert a version's string into a vector<string> with each comparable part
    QI_API std::vector<std::string> explode(const std::string &version);

    //compare two version's strings. a < b return -1
    QI_API int                      compare(const std::string &versionA,
                                            const std::string &versionB);

    QI_API std::string              extract(const std::string &version);

  }
}

#endif  // _QI_VERSION_HPP_
