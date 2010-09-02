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

      template<typename T0>
      void callVoid(const std::string methodName, const T0& arg0) {
        xCall(AL::Messaging::CallDefinition(methodName, arg0));
      }

      template<typename T0, typename T1>
      void callVoid(const std::string methodName, const T0& arg0, const T1& arg1) {
        xCall(AL::Messaging::CallDefinition(methodName, arg0, arg1));
      }

      template<typename T0, typename T1, typename T2>
      void callVoid(const std::string methodName, const T0& arg0, const T1& arg1, const T2& arg2) {
        xCall(AL::Messaging::CallDefinition(methodName, arg0, arg1, arg2));
      }

      template<typename T0, typename T1, typename T2, typename T3>
      void callVoid(const std::string methodName, const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3) {
        xCall(AL::Messaging::CallDefinition(methodName, arg0, arg1, arg2, arg3));
      }

      template<typename T0, typename T1, typename T2, typename T3, typename T4>
      void callVoid(const std::string methodName, const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4) {
        xCall(AL::Messaging::CallDefinition(methodName, arg0, arg1, arg2, arg3, arg4));
      }

      template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
      void callVoid(const std::string methodName, const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5) {
        xCall(AL::Messaging::CallDefinition(methodName, arg0, arg1, arg2, arg3, arg4, arg5));
      }

      //

      template<typename R>
      R call(const std::string methodName) {
        AL::Messaging::ResultDefinition ret = xCall(AL::Messaging::CallDefinition(methodName));
        return ret.value().as<R>();
      }

      template<typename R, typename T0>
      R call(const std::string methodName, const T0& arg0) {
        AL::Messaging::ResultDefinition ret = xCall(AL::Messaging::CallDefinition(methodName, arg0));
        return ret.value().as<R>();
      }

      template<typename R, typename T0, typename T1>
      R call(const std::string methodName, const T0& arg0, const T1& arg1) {
        AL::Messaging::ResultDefinition ret = xCall(AL::Messaging::CallDefinition(methodName, arg0, arg1));
        return ret.value().as<R>();
      }

      template<typename R, typename T0, typename T1, typename T2>
      R call(const std::string methodName, const T0& arg0, const T1& arg1, const T2& arg2) {
        AL::Messaging::ResultDefinition ret = xCall(AL::Messaging::CallDefinition(methodName, arg0, arg1, arg2));
        return ret.value().as<R>();
      }

      template<typename R, typename T0, typename T1, typename T2, typename T3>
      R call(const std::string methodName, const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3) {
        AL::Messaging::ResultDefinition ret = xCall(AL::Messaging::CallDefinition(methodName, arg0, arg1, arg2, arg3));
        return ret.value().as<R>();
      }

      template<typename R, typename T0, typename T1, typename T2, typename T3, typename T4>
      R call(const std::string methodName, const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4) {
        AL::Messaging::ResultDefinition ret = xCall(AL::Messaging::CallDefinition(methodName, arg0, arg1, arg2, arg3, arg4));
        return ret.value().as<R>();
      }

      template<typename R, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
      R call(const std::string methodName, const T0& arg0, const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5) {
        AL::Messaging::ResultDefinition ret = xCall(AL::Messaging::CallDefinition(methodName, arg0, arg1, arg2, arg3, arg4, arg5));
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
      boost::shared_ptr<ClientNodeImp> fImp;
    };
  }
}

#endif  // COMMON_CLIENT_NODE_HPP_

