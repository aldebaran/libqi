/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/node.hpp>

namespace qi {
  /// <summary> Default constructor. </summary>
  Node::Node(): Server(), Client() {}
  Node::~Node() {}

  /// <summary> Full constructor. </summary>
  /// <param name="name"> Name of the node. </param>
  /// <param name="serverAddress"> The server address. </param>
  /// <param name="masterAddress"> The master address. </param>
  Node::Node(
    const std::string& name,
    const std::string& serverAddress,
    const std::string& masterAddress) :
        Server(name, serverAddress, masterAddress),
        Client(name, masterAddress) {}
}
