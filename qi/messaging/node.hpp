#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_NODE_HPP_
#define _QI_MESSAGING_NODE_HPP_

#include <qi/messaging/server.hpp>
#include <qi/messaging/client.hpp>
#include <qi/messaging/publisher.hpp>
#include <qi/messaging/subscriber.hpp>

namespace qi {
  /// <summary>
  /// Node: A combination of Server, Client, Publisher and
  /// Subscriber
  /// </summary>
  /// \ingroup Messaging
  class Node :
    public Server,
    public Client,
    public Publisher,
    public Subscriber
  {
  public:
    /// <summary> Finaliser. </summary>
    virtual ~Node();

    /// <summary> Creates a Node </summary>
    /// <param name="nodeName"> Name of the node. Default: "Node" </param>
    /// <param name="masterAddress"> The master address. </param>
    Node(const std::string& nodeName = "", Context *ctx = 0);

    /// <summary> Reset to the default state, this will disconnect
    /// and reset the object, like a new fresh copy. </summary>
    /// <param name="name"> Name </param>
    /// <param name="context"> an optional Context </param>
    void reset(const std::string &name = "", Context *ctx = 0);

    /// <summary> Connect to masterAddress. If no address is specified
    /// the default 127.0.0.1:5555 is used </summary>
    /// <param name="masterAddress"> The master address. </param>
    void connect(const std::string &masterAddress = "127.0.0.1:5555");

  };
}

#endif  // _QI_MESSAGING_NODE_HPP_

