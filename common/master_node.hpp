#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef AL_COMMON_MASTER_NODE_HPP_
#define AL_COMMON_MASTER_NODE_HPP_

#include <string>
#include <memory>

namespace AL {
  namespace Common {

    class MasterNodeImp;

    class MasterNode {
    public:
      explicit MasterNode(const std::string& masterAddress);
      virtual ~MasterNode();

    private:
      std::auto_ptr<MasterNodeImp> fImp;
    };
  }
}

#endif  // AL_COMMON_MASTER_NODE_HPP_

