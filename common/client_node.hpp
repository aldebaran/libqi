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

      void callVoid(const std::string methodName) {
        xCall(AL::Messaging::CallDefinition(methodName));
      }

      template<typename R, typename T0>
      R call(const std::string methodName, const T0& arg0) {
        AL::Messaging::ResultDefinition ret = xCall(AL::Messaging::CallDefinition(methodName, arg0));
        return ret.value().as<R>();
      }

      //// ---- TODO template these ---
      //void call(const std::string& methodName,
      //  const AL::Messaging::ArgumentList& params,
      //  AL::Messaging::ReturnValue& result);

      //void call(const std::string& methodName,
      //  AL::Messaging::ReturnValue& result);

      //AL::Messaging::ReturnValue call(const std::string& methodName,
      //  const AL::Messaging::ArgumentList& params);

      //AL::Messaging::ReturnValue call(const std::string& methodName);
      //// -----------------------------

    private:
      AL::Messaging::ResultDefinition xCall(const AL::Messaging::CallDefinition& callDef);
      void ClientNode::xCallVoid(const AL::Messaging::CallDefinition& callDef);
      boost::shared_ptr<ClientNodeImp> fImp;
    };
  }
}

#endif  // COMMON_CLIENT_NODE_HPP_

