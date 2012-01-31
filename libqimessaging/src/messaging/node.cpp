/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qimessaging/node.hpp>
#include <qimessaging/context.hpp>

namespace qi {
  Node::~Node() {}

  Node::Node(const std::string& name, Context *ctx)
    : Server(name, ctx),
      Client(name, ctx)
  {
  }


}
