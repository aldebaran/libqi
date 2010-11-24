#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_SERVER_HPP__
#define   __QI_MESSAGING_SERVER_HPP__

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

    /// <summary> Constructs a Server object that can be used to
    /// advertise services to clients. </summary>
    /// <param name="serverName"> Name of the server. </param>
    /// <param name="masterAddress"> The master address. </param>
    Server(const std::string& serverName,
           const std::string& masterAddress = "127.0.0.1:5555");

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
    /// <param name="object"> The memory address of the object. This could be
    /// a "this" pointer if you are adding a method of your class,
    /// or a "&myObject" for the address of a member object.</param>
    /// <param name="method"> The memory address of the method. This should fully
    /// qualify the type of your method such as "&MyClass::myMethod" or
    /// "&MyObject::objectMethod".</param>
    template <typename OBJECT_TYPE, typename METHOD_TYPE>
    void addService(const std::string& name, OBJECT_TYPE object, METHOD_TYPE method) {
      xAddService(makeSignature(name, method), makeFunctor(object, method));
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
    /// <param name="function"> The memory address of the function. e.g. "&globalFunction"</param>
    template <typename FUNCTION_TYPE>
    void addService(const std::string& name, FUNCTION_TYPE function) {
      xAddService(makeSignature(name, function), makeFunctor(function));
    }

    //template<typename TOPIC_TYPE>
    //Publisher addTopic(std::string topicName);

    bool isInitialized() const;

  private:
    void xAddService(const std::string& methodSignature, qi::Functor* functor);
    std::auto_ptr<detail::ServerImpl> _impl;
  };

}

#endif // __QI_MESSAGING_SERVER_HPP__

