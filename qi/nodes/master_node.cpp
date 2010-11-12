/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/nodes/master_node.hpp>
#include <string>
#include <qi/nodes/detail/master_node_imp.hpp>

namespace qi {
  /// <summary> Constructor. </summary>
  /// <param name="masterAddress"> The master address. </param>
  MasterNode::MasterNode(const std::string& masterAddress) :
    fImp(new detail::MasterNodeImp(masterAddress)) {}

  /// <summary> Destructor. </summary>
  MasterNode::~MasterNode() {}
}
