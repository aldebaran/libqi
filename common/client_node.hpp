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
#include <alcommon-ng/collections/variables_list.hpp>
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

      void call(const std::string& methodName,
        const AL::Messaging::ArgumentList& params,
        AL::Messaging::ReturnValue& result);

      void call(const std::string& methodName,
        AL::Messaging::ReturnValue& result);

      AL::Messaging::ReturnValue call(const std::string& methodName,
        const AL::Messaging::ArgumentList& params);

      AL::Messaging::ReturnValue call(const std::string& methodName);

    private:
      boost::shared_ptr<ClientNodeImp> fImp;
    };
  }
}

#endif  // COMMON_CLIENT_NODE_HPP_

