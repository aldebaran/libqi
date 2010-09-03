/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <alcommon-ng/common/master_node.hpp>
#include <string>
#include <alcommon-ng/common/detail/master_node_imp.hpp>

namespace AL {
  namespace Common {

    /// <summary> Constructor. </summary>
    /// <param name="masterAddress"> The master address. </param>
    MasterNode::MasterNode(const std::string& masterAddress) :
      fImp(new MasterNodeImp(masterAddress)) {}

    /// <summary> Destructor. </summary>
    MasterNode::~MasterNode() {}
  }
}
