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
#include <boost/shared_ptr.hpp>

namespace AL {
  namespace Common {

    // forward declared implementation
    class MasterNodeImp;

    class MasterNode {
    public:
      MasterNode(const std::string& masterAddress);

    private:
      boost::shared_ptr<MasterNodeImp> fImp;
    };
  }
}

#endif  // AL_COMMON_MASTER_NODE_HPP_

