/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/master.hpp>
#include <string>
#include <qi/messaging/detail/master_impl.hpp>

namespace qi {
  /// <summary> Constructor. </summary>
  /// <param name="masterAddress"> The master address. </param>
  Master::Master(const std::string& masterAddress) :
    _impl(new detail::MasterImpl(masterAddress)) {}

  /// <summary> Destructor. </summary>
  Master::~Master() {}
}
