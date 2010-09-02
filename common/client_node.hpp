
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
#include <alcommon-ng/messaging/call_definition.hpp>
#include <alcommon-ng/messaging/result_definition.hpp>
#include <alcommon-ng/functor/functionsignature.hpp>
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
          AL::Messaging::ResultDefinition result;
          void (*f)()  = 0;
          std::string hash = makeSignature(methodName, f);
          xCall(AL::Messaging::CallDefinition(hash), result);
      }

      template <typename R>
      R call(const std::string methodName) {
        AL::Messaging::ResultDefinition result;
        R (*f)()  = 0;
        std::string hash = makeSignature(methodName, f);
        xCall(AL::Messaging::CallDefinition(hash), result);
        return result.value().as<R>();
      }

      template <typename P0>
      void callVoid(const std::string methodName, const P0 &p0) {
          AL::Messaging::ResultDefinition result;
          void (*f)(const P0 &p0)  = 0;
          std::string hash = makeSignature(methodName, f);
          xCall(AL::Messaging::CallDefinition(hash, p0), result);
      }

      template <typename R, typename P0>
      R call(const std::string methodName, const P0 &p0) {
        AL::Messaging::ResultDefinition result;
        R (*f)(const P0 &p0)  = 0;
        std::string hash = makeSignature(methodName, f);
        xCall(AL::Messaging::CallDefinition(hash, p0), result);
        return result.value().as<R>();
      }

      template <typename P0, typename P1>
      void callVoid(const std::string methodName, const P0 &p0, const P1 &p1) {
          AL::Messaging::ResultDefinition result;
          void (*f)(const P0 &p0, const P1 &p1)  = 0;
          std::string hash = makeSignature(methodName, f);
          xCall(AL::Messaging::CallDefinition(hash, p0, p1), result);
      }

      template <typename R, typename P0, typename P1>
      R call(const std::string methodName, const P0 &p0, const P1 &p1) {
        AL::Messaging::ResultDefinition result;
        R (*f)(const P0 &p0, const P1 &p1)  = 0;
        std::string hash = makeSignature(methodName, f);
        xCall(AL::Messaging::CallDefinition(hash, p0, p1), result);
        return result.value().as<R>();
      }

      template <typename P0, typename P1, typename P2>
      void callVoid(const std::string methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
          AL::Messaging::ResultDefinition result;
          void (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
          std::string hash = makeSignature(methodName, f);
          xCall(AL::Messaging::CallDefinition(hash, p0, p1, p2), result);
      }

      template <typename R, typename P0, typename P1, typename P2>
      R call(const std::string methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
        AL::Messaging::ResultDefinition result;
        R (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
        std::string hash = makeSignature(methodName, f);
        xCall(AL::Messaging::CallDefinition(hash, p0, p1, p2), result);
        return result.value().as<R>();
      }

      template <typename P0, typename P1, typename P2, typename P3>
      void callVoid(const std::string methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
          AL::Messaging::ResultDefinition result;
          void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
          std::string hash = makeSignature(methodName, f);
          xCall(AL::Messaging::CallDefinition(hash, p0, p1, p2, p3), result);
      }

      template <typename R, typename P0, typename P1, typename P2, typename P3>
      R call(const std::string methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
        AL::Messaging::ResultDefinition result;
        R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
        std::string hash = makeSignature(methodName, f);
        xCall(AL::Messaging::CallDefinition(hash, p0, p1, p2, p3), result);
        return result.value().as<R>();
      }

      template <typename P0, typename P1, typename P2, typename P3, typename P4>
      void callVoid(const std::string methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
          AL::Messaging::ResultDefinition result;
          void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
          std::string hash = makeSignature(methodName, f);
          xCall(AL::Messaging::CallDefinition(hash, p0, p1, p2, p3, p4), result);
      }

      template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4>
      R call(const std::string methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
        AL::Messaging::ResultDefinition result;
        R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
        std::string hash = makeSignature(methodName, f);
        xCall(AL::Messaging::CallDefinition(hash, p0, p1, p2, p3, p4), result);
        return result.value().as<R>();
      }

      template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
      void callVoid(const std::string methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
          AL::Messaging::ResultDefinition result;
          void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
          std::string hash = makeSignature(methodName, f);
          xCall(AL::Messaging::CallDefinition(hash, p0, p1, p2, p3, p4, p5), result);
      }

      template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
      R call(const std::string methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
        AL::Messaging::ResultDefinition result;
        R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
        std::string hash = makeSignature(methodName, f);
        xCall(AL::Messaging::CallDefinition(hash, p0, p1, p2, p3, p4, p5), result);
        return result.value().as<R>();
      }

    private:
      void xCall(const AL::Messaging::CallDefinition& callDef, AL::Messaging::ResultDefinition &result);
      //TODO optimise : autoptr is faster
      boost::shared_ptr<ClientNodeImp> fImp;
    };
  }
}
#endif  // COMMON_CLIENT_NODE_HPP_
