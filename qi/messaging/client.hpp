
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
#ifndef   __QI_MESSAGING_CLIENT_HPP_IN__
#define   __QI_MESSAGING_CLIENT_HPP_IN__

#include <string>
#include <memory>
#include <qi/signature.hpp>
#include <qi/serialization/serializer.hpp>

namespace qi {
  namespace detail {
    class ClientImpl;
  }

  /// <summary>
  /// Used to call services that have been added to a server.
  /// If the service is unknown, the master is interogated
  ///  to find the appropriate server
  /// </summary>
  class Client {
  public:
    /// <summary>
    /// DefaultConstructor
    /// Used to call services that have been added to a server.
    /// If the service is unknown, the master is interogated
    ///  to find the appropriate server
    /// </summary>
    Client();

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
    Client(const std::string& clientName, const std::string& masterAddress);

    virtual ~Client();

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
    void xCall(const std::string &signature, const qi::serialization::BinarySerializer &callDef, qi::serialization::BinarySerializer &result);
    std::auto_ptr<detail::ClientImpl> _impl;
  };
}

#include <qi/messaging/client.hxx>
#endif // __QI_MESSAGING_CLIENT_HPP_IN__
