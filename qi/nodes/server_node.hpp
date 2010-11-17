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
#include <memory>
#include <qi/functors/makefunctor.hpp>
#include <qi/signature.hpp>

namespace qi {
  namespace detail {
    class ServerNodeImp;
  }

  /// <summary> Server node. </summary>
  class ServerNode {
  public:

    /// <summary> Default constructor. </summary>
    ServerNode();

    /// <summary> Finaliser. </summary>
    virtual ~ServerNode();

    /// <summary> Default constructor. </summary>
    /// <param name="nodeName"> Name of the node. </param>
    /// <param name="nodeAddress"> The node address. </param>
    /// <param name="masterAddress"> The master address. </param>
    ServerNode(const std::string& nodeName,
               const std::string& nodeAddress,
               const std::string& masterAddress);

    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    void addService(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method) {
      xAddService(makeSignature(name, method), makeFunctor(object, method));
    }

    template <typename FUNCTION_TYPE>
    void addService(const std::string& name, FUNCTION_TYPE function) {
      xAddService(makeSignature(name, function), makeFunctor(function));
    }

  private:
    void xAddService(const std::string& methodSignature, qi::Functor* functor);
    std::auto_ptr<detail::ServerNodeImp> fImp;
  };

}

#endif  // COMMON_SERVER_NODE_HPP_

