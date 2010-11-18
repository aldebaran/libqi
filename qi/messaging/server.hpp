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
    class ServerImpl;
  }

  /// <summary> Server. </summary>
  class Server {
  public:

    /// <summary> Default constructor. </summary>
    Server();

    /// <summary> Finaliser. </summary>
    virtual ~Server();

    /// <summary> Default constructor. </summary>
    /// <param name="serverName"> Name of the server. </param>
    /// <param name="serverAddress"> The server address. </param>
    /// <param name="masterAddress"> The master address. </param>
    Server(const std::string& serverName,
               const std::string& serverAddress,
               const std::string& masterAddress);

    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    void addService(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method) {
      xAddService(makeSignature(name, method), makeFunctor(object, method));
    }

    template <typename FUNCTION_TYPE>
    void addService(const std::string& name, FUNCTION_TYPE function) {
      xAddService(makeSignature(name, function), makeFunctor(function));
    }

    //template<typename TOPIC_TYPE>
    //Publisher addTopic(std::string topicName);

  private:
    void xAddService(const std::string& methodSignature, qi::Functor* functor);
    std::auto_ptr<detail::ServerImpl> fImp;
  };

}

#endif  // COMMON_SERVER_NODE_HPP_

