
#pragma once
/*
** $autogen
**
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**  - Cedric Gestes <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef COMMON_CLIENT_NODE_HPP_
#define COMMON_CLIENT_NODE_HPP_

#include <string>
#include <memory>
#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/messaging/result_definition.hpp>
#include <alcommon-ng/functor/functionsignature.hpp>

namespace AL {
  namespace Common {

    class ClientNodeImp;

    /// <summary>
    /// Used to call services that have been added to a server.
    /// If the service is unknown, the master is interogated
    //  to find the appropriate server
    /// </summary>
    class ClientNode {
    public:

      /// <summary>
      /// DefaultConstructor
      /// Used to call services that have been added to a server.
      /// If the service is unknown, the master is interogated
      //  to find the appropriate server
      /// </summary>
      ClientNode();

      /// <summary>
      /// Used to call services that have been added to a server.
      /// If the service is unknown, the master is interogated
      //  to find the appropriate server
      /// </summary>
      /// <param name="clientName">
      /// The name you want to give to this client
      /// e.g. "client"
      /// </param>
      /// <param name="masterAddress">
      /// The address of the master that is used to find services
      /// e.g. "127.0.0.1:5555"
      /// </param>
      ClientNode(const std::string& clientName,
        const std::string& masterAddress);

      virtual ~ClientNode();

      /// <summary>
      /// A call with no args other than methodName and a return type of void
      /// </summary>
      /// <param name="methodName">
      /// The name of the method you want to call
      /// </param>
      void callVoid(const std::string& methodName);

      template <typename RETURN_TYPE>
      RETURN_TYPE call(const std::string& methodName);

      template <typename P0>
      void callVoid(const std::string& methodName, const P0 &p0);

      template <typename RETURN_TYPE, typename P0>
      RETURN_TYPE call(const std::string& methodName, const P0 &p0);

      template <typename P0, typename P1>
      void callVoid(const std::string& methodName, const P0 &p0, const P1 &p1);

      template <typename RETURN_TYPE, typename P0, typename P1>
      RETURN_TYPE call(const std::string& methodName, const P0 &p0, const P1 &p1);

      template <typename P0, typename P1, typename P2>
      void callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2);

      template <typename RETURN_TYPE, typename P0, typename P1, typename P2>
      RETURN_TYPE call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2);

      template <typename P0, typename P1, typename P2, typename P3>
      void callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3);

      template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3>
      RETURN_TYPE call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3);

      template <typename P0, typename P1, typename P2, typename P3, typename P4>
      void callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4);

      template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4>
      RETURN_TYPE call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4);

      template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
      void callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5);

      template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
      RETURN_TYPE call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5);

    private:
      void xCall(const AL::Messaging::CallDefinition& callDef, AL::Messaging::ResultDefinition &result);
      std::auto_ptr<ClientNodeImp> fImp;
    };
  }
}

#include <alcommon-ng/common/client_node.hxx>
#endif  // COMMON_CLIENT_NODE_HPP_
