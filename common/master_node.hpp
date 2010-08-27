#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef AL_COMMON_MASTER_NODE_H_
#define AL_COMMON_MASTER_NODE_H_

#include <string>
#include <alcommon-ng/common/server_node.hpp>
#include <alcommon-ng/common/client_node.hpp>

namespace AL {
  namespace Common {

    class MasterNode :
      public AL::Common::ServerNode {
    public:
      MasterNode(const std::string& masterName, const std::string& masterAddress);
    };
  }
}

#endif  // AL_COMMON_MASTER_NODE_H_

