#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_NODE_HPP__
#define   __QI_MESSAGING_NODE_HPP__

#include <qi/messaging/server.hpp>
#include <qi/messaging/client.hpp>
#include <qi/messaging/publisher.hpp>
#include <qi/messaging/subscriber.hpp>

namespace qi {
  /// <summary> Node: A combination of Server, Client, Publisher and Subscriber </summary>
  class Node : public Server, public Client, public Publisher, public Subscriber {
  public:
    /// <summary> Finaliser. </summary>
    virtual ~Node();

    /// <summary> Creates a Node </summary>
    /// <param name="nodeName"> Name of the node. Default: "Node" </param>
    /// <param name="masterAddress"> The master address. </param>
    Node(const std::string& nodeName = "node",
         const std::string& masterAddress = "127.0.0.1:5555");
  };
}

#endif // __QI_MESSAGING_NODE_HPP__

