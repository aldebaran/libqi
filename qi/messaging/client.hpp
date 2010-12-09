#pragma once
/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_CLIENT_HPP_
#define _QI_MESSAGING_CLIENT_HPP_

#include <string>
#include <qi/signature.hpp>
#include <qi/serialization/serializer.hpp>
#include <boost/scoped_ptr.hpp>

namespace qi {
  namespace detail {
    class ClientImpl;
  }

  class Context;

  /// <summary>
  /// Used to call services that have been added to a server.
  /// If the service is unknown, the master is interrogated
  /// to find the appropriate server.
  /// </summary>
  /// \ingroup Messaging
  class Client {
  public:
    /// <summary>
    /// Used to call services that have been added to a server.
    /// If the service is unknown, the master is interrogated
    /// to find the appropriate server
    /// </summary>
    /// <param name="name">
    /// The name you want to give to this client
    /// e.g. "client"
    /// </param>
    /// <param name="context">
    /// An optional context that can be used to group or separate
    /// transport resources.
    /// </param>
    Client(const std::string& name = "", qi::Context *context = 0);

    /// <summary> Reset to the default state, this will disconnect
    /// and reset the object, like a new fresh copy. </summary>
    /// <param name="name"> Name </param>
    /// <param name="context">
    /// An optional context that can be used to group or separate
    /// transport resources.
    /// </param>
    void reset(const std::string &name = "", qi::Context *context = 0);

    /// <summary> Connect to masterAddress. If no address is specified
    /// the default 127.0.0.1:5555 is used </summary>
    /// <param name="masterAddress"> The master address. e.g 127.0.0.1:5555</param>
    void connect(const std::string &masterAddress = "127.0.0.1:5555");

    virtual ~Client();

    bool isInitialized() const;

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

    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    void callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6);

    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
    RETURN_TYPE call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6);

    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    void callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7);

    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
    RETURN_TYPE call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7);

    template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    void callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8);

    template <typename RETURN_TYPE, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
    RETURN_TYPE call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8);

  private:
    void xCall(const std::string& signature,
      const qi::serialization::Message& request,
            qi::serialization::Message& result);
    boost::scoped_ptr<qi::detail::ClientImpl> _impl;
  };
}

#include <qi/messaging/client.hxx>
#endif  // _QI_MESSAGING_CLIENT_HPP_
