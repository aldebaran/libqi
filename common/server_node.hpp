#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_SERVER_NODE_HPP_
#define COMMON_SERVER_NODE_HPP_

#include <string>
#include <alcommon-ng/common/serviceinfo.hpp>
#include <boost/shared_ptr.hpp>

namespace AL {
  namespace Common {

    // forward declared implementation
    class ServerNodeImp;

    class ServerNode {
    public:
      ServerNode();
      ServerNode(const std::string& nodeName,
        const std::string& nodeAddress,
        const std::string& masterAddress);

      void addService(const ServiceInfo& service);

      template <typename P0, typename C, typename R>
      void addService(const std::string& name, C *obj, R (C::*f) (const P0 &p0)) {
        addService(ServiceInfo(name, makeFunctor(C *obj, R (C::*f) (const P0 &p0))));
      }

    private:
      boost::shared_ptr<ServerNodeImp> fImp;
    };
  }
}

#endif  // COMMON_SERVER_NODE_HPP_

