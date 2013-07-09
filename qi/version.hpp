#pragma once
/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_VERSION_HPP_
#define _QI_VERSION_HPP_

#include <boost/shared_ptr.hpp>
#include <qi/api.hpp>
#include <vector>
#include <string>

namespace qi {
  class VersionPrivate;
  class QI_API Version
  {
    public:
      Version();
      Version(const std::string& version);
      Version(const char* version);

      const std::string& toString() const;

      bool operator< (const Version& rhs) const;
      bool operator> (const Version& rhs) const;
      bool operator==(const Version& rhs) const;
      bool operator!=(const Version& rhs) const;
      bool operator<=(const Version& rhs) const;
      bool operator>=(const Version& rhs) const;

    private:
      boost::shared_ptr<VersionPrivate> _p;
  };
}

#endif  // _QI_VERSION_HPP_
