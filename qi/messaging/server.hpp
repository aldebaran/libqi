#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_SERVER_HPP_
#define _QI_MESSAGING_SERVER_HPP_

#include <string>
#include <boost/scoped_ptr.hpp>
#include <qi/functors/makefunctor.hpp>
#include <qi/signature.hpp>

namespace qi {
  namespace detail {
    class ServerImpl;
  }

  class Context;

  /// <summary> Used to advertise named services. Advertised Services are
  /// registered with the master so that clients can find them.</summary>
  /// \b Advertise a Service
  /// \include example_qi_server.cpp
  /// \ingroup Messaging
  class Server {
  public:

    /// <summary> Finaliser. </summary>
    virtual ~Server();

    /// <summary> Constructs a Server object that can be used to
    /// advertise services to clients. </summary>
    /// <param name="name"> The name you want to give to the server. </param>
    /// <param name="context">
    /// An optional context that can be used to group or separate
    /// transport resources.
    /// </param>
    Server(const std::string &name = "", qi::Context *context = 0);

    /// <summary> Connect to masterAddress. If no address is specified
    /// the default 127.0.0.1:5555 is used </summary>
    /// <param name="masterAddress"> The master address. </param>
    void connect(const std::string &masterAddress = "127.0.0.1:5555");

    /// <summary>
    /// Makes an object's method available as a service, and registers
    /// this service with the master so that clients can find it.
    ///
    /// The method's call signature is found by introspection and
    /// combined with the service name to create a service signature.
    /// This signature is used to find your method and to serialize
    /// and deserialize outgoing and incoming data.
    ///
    /// e.g. server.addService("hello", this, &MyClass::helloMethod);
    ///
    /// e.g. server.addService("hello", &myObject, &MyObject::helloMethod);
    /// </summary>
    /// <param name="name"> The advertised name of the service.</param>
    /// <param name="object"> The memory address of the object. This could
    /// be a "this" pointer if you are adding a method of your class,
    /// or a "&myObject" for the address of a member object.</param>
    /// <param name="method"> The memory address of the method. This should
    /// fully qualify the type of your method such as "&MyClass::myMethod"
    /// or "&MyObject::objectMethod".</param>
    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    void advertiseService(
      const std::string& name, OBJECT_TYPE object, METHOD_TYPE method)
    {
      xAdvertiseService(
        makeFunctionSignature(name, method),
        makeFunctor(object, method));
    }

    /// <summary>
    /// Makes a function available as a service, and registers
    /// this service with the master so that clients can find it.
    ///
    /// The method's call signature is found by introspection and
    /// combined with the service name to create a service signature.
    /// This signature is used to find your method and to serialize
    /// and deserialize outgoing and incoming data.
    ///
    /// e.g. server.addService("hello", &helloFunction);
    /// </summary>
    /// <param name="name"> The advertised name of the service.</param>
    /// <param name="function"> The memory address of the function. e.g.
    /// "&globalFunction"
    /// </param>
    template <typename FUNCTION_TYPE>
    void advertiseService(const std::string& name, FUNCTION_TYPE function)
    {
      xAdvertiseService(makeFunctionSignature(name, function), makeFunctor(function));
    }

    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    void unadvertiseService(
      const std::string& name, OBJECT_TYPE object, METHOD_TYPE method)
    {
      xUnadvertiseService(makeFunctionSignature(name, method));
    }

    template <typename FUNCTION_TYPE>
    void unadvertiseService(const std::string& name, FUNCTION_TYPE function)
    {
      xUnadvertiseService(makeFunctionSignature(name, function));
    }

    bool isInitialized() const;

  private:

    /// <summary> Private method that advertises a service </summary>
    /// <param name="methodSignature">The method signature.</param>
    /// <param name="functor"> The functor that handles the messages</param>
    void xAdvertiseService(const std::string& methodSignature, qi::Functor* functor);

    /// <summary>Private method that unadvertises a service. </summary>
    /// <param name="methodSignature">The method signature.</param>
    void xUnadvertiseService(const std::string& methodSignature);

    /// <summary> The private implementation </summary>
    boost::scoped_ptr<detail::ServerImpl> _impl;
  };
}

#endif  // _QI_MESSAGING_SERVER_HPP_
