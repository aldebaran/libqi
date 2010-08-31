/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/master_node.hpp>
#include <alcommon-ng/common/detail/master_node_imp.hpp>

namespace AL {
  namespace Common {

    MasterNode::MasterNode(
      const std::string& masterAddress) :
    fImp(boost::shared_ptr<MasterNodeImp>(new MasterNodeImp(masterAddress))) {}

  }
}
