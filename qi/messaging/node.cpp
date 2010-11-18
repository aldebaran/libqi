/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/messaging/node.hpp>

namespace qi {
  Node::Node(): Server(), Client() {}

  Node::~Node() {}

  Node::Node(
    const std::string& name,
    const std::string& serverAddress,
    const std::string& masterAddress) :
        Server(name, serverAddress, masterAddress),
        Client(name, masterAddress) {}
}
