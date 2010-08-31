#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_CLIENT_NODE_HPP_
#define COMMON_CLIENT_NODE_HPP_

#include <string>
#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/messaging/result_definition.hpp>
#include <boost/shared_ptr.hpp>

namespace AL {
  namespace Common {

    // forward declared implementation
    class ClientNodeImp;

    class ClientNode {
    public:
      ClientNode();

      ClientNode(const std::string& clientName,
        const std::string& masterAddress);

      virtual ~ClientNode();

      AL::Messaging::ResultDefinition call(
        const AL::Messaging::CallDefinition& callDef);

    private:
      boost::shared_ptr<ClientNodeImp> fImp;
    };
  }
}

#endif  // COMMON_CLIENT_NODE_HPP_

