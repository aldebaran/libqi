
#pragma once
/*
** $autogen
**
** Author(s):
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**  - Cedric Gestes <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef QI_NODES_CLIENT_NODE_HXX_
#define QI_NODES_CLIENT_NODE_HXX_

namespace qi {


  template <typename RETURN_TYPE>
  RETURN_TYPE ClientNode::call(const std::string& methodName) {
    qi::messaging::ResultDefinition result;
    RETURN_TYPE (*f)()  = 0;
    std::string hash = makeSignature(methodName, f);
    xCall(qi::messaging::CallDefinition(hash), result);
    return result.value().as<RETURN_TYPE>();
  }

  template <typename P0>
  void ClientNode::callVoid(const std::string& methodName, const P0 &p0) {
      qi::messaging::ResultDefinition result;
      void (*f)(const P0 &p0)  = 0;
      std::string hash = makeSignature(methodName, f);
      xCall(qi::messaging::CallDefinition(hash, p0), result);
  }

  template <typename RETURN_TYPE, typename P0>
  RETURN_TYPE ClientNode::call(const std::string& methodName, const P0 &p0) {
    qi::messaging::ResultDefinition result;
    RETURN_TYPE (*f)(const P0 &p0)  = 0;
    std::string hash = makeSignature(methodName, f);
    xCall(qi::messaging::CallDefinition(hash, p0), result);
    return result.value().as<RETURN_TYPE>();
  }

  template <typename P0, typename P1>
  void ClientNode::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1) {
      qi::messaging::ResultDefinition result;
      void (*f)(const P0 &p0, const P1 &p1)  = 0;
      std::string hash = makeSignature(methodName, f);
      xCall(qi::messaging::CallDefinition(hash, p0, p1), result);
  }

  template <typename RETURN_TYPE, typename P0, typename P1>
  RETURN_TYPE ClientNode::call(const std::string& methodName, const P0 &p0, const P1 &p1) {
    qi::messaging::ResultDefinition result;
    RETURN_TYPE (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string hash = makeSignature(methodName, f);
    xCall(qi::messaging::CallDefinition(hash, p0, p1), result);
    return result.value().as<RETURN_TYPE>();
  }

  template <typename P0, typename P1, typename P2>
  void ClientNode::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
      qi::messaging::ResultDefinition result;
      void (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
      std::string hash = makeSignature(methodName, f);
      xCall(qi::messaging::CallDefinition(hash, p0, p1, p2), result);
  }

  template <typename RETURN_TYPE, typename P0, typename P1, typename P2>
  RETURN_TYPE ClientNode::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::messaging::ResultDefinition result;
    RETURN_TYPE (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string hash = makeSignature(methodName, f);
    xCall(qi::messaging::CallDefinition(hash, p0, p1, p2), result);
    return result.value().as<RETURN_TYPE>();
  }

  template <typename P0, typename P1, typename P2, typename P3>
  void ClientNode::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
      qi::messaging::ResultDefinition result;
      void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
      std::string hash = makeSignature(methodName, f);
      xCall(qi::messaging::CallDefinition(hash, p0, p1, p2, p3), result);
  }

  template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3>
  RETURN_TYPE ClientNode::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::messaging::ResultDefinition result;
    RETURN_TYPE (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string hash = makeSignature(methodName, f);
    xCall(qi::messaging::CallDefinition(hash, p0, p1, p2, p3), result);
    return result.value().as<RETURN_TYPE>();
  }

  template <typename P0, typename P1, typename P2, typename P3, typename P4>
  void ClientNode::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
      qi::messaging::ResultDefinition result;
      void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
      std::string hash = makeSignature(methodName, f);
      xCall(qi::messaging::CallDefinition(hash, p0, p1, p2, p3, p4), result);
  }

  template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4>
  RETURN_TYPE ClientNode::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::messaging::ResultDefinition result;
    RETURN_TYPE (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string hash = makeSignature(methodName, f);
    xCall(qi::messaging::CallDefinition(hash, p0, p1, p2, p3, p4), result);
    return result.value().as<RETURN_TYPE>();
  }

  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  void ClientNode::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
      qi::messaging::ResultDefinition result;
      void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
      std::string hash = makeSignature(methodName, f);
      xCall(qi::messaging::CallDefinition(hash, p0, p1, p2, p3, p4, p5), result);
  }

  template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  RETURN_TYPE ClientNode::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::messaging::ResultDefinition result;
    RETURN_TYPE (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string hash = makeSignature(methodName, f);
    xCall(qi::messaging::CallDefinition(hash, p0, p1, p2, p3, p4, p5), result);
    return result.value().as<RETURN_TYPE>();
  }

}
#endif  // QI_NODES_CLIENT_NODE_HXX_
