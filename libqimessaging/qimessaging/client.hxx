
#pragma once
/*
* $autogen
*
*  Author(s):
*  - Chris Kilner  <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QIMESSAGING_CLIENT_HXX_
#define _QIMESSAGING_CLIENT_HXX_

#include <qimessaging/serialization.hpp>

namespace qi {


  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R>
  R Client::call(const std::string& methodName) {
    qi::Message request;
    qi::Message reply;

    R (*f)()  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(reply, ret);
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0>
  void Client::callVoid(const std::string& methodName, const P0 &p0) {
    qi::Message request;
    qi::Message reply;

    void (*f)(const P0 &p0)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0>
  R Client::call(const std::string& methodName, const P0 &p0) {
    qi::Message request;
    qi::Message reply;

    R (*f)(const P0 &p0)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(reply, ret);
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1) {
    qi::Message request;
    qi::Message reply;

    void (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1) {
    qi::Message request;
    qi::Message reply;

    R (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(reply, ret);
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::Message request;
    qi::Message reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::Message request;
    qi::Message reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(reply, ret);
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::Message request;
    qi::Message reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::Message request;
    qi::Message reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(reply, ret);
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::Message request;
    qi::Message reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    qi::serialization::serialize<P4>::write(request, p4);
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::Message request;
    qi::Message reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    qi::serialization::serialize<P4>::write(request, p4);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(reply, ret);
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::Message request;
    qi::Message reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    qi::serialization::serialize<P4>::write(request, p4);
    qi::serialization::serialize<P5>::write(request, p5);
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::Message request;
    qi::Message reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    qi::serialization::serialize<P4>::write(request, p4);
    qi::serialization::serialize<P5>::write(request, p5);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(reply, ret);
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) {
    qi::Message request;
    qi::Message reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    qi::serialization::serialize<P4>::write(request, p4);
    qi::serialization::serialize<P5>::write(request, p5);
    qi::serialization::serialize<P6>::write(request, p6);
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) {
    qi::Message request;
    qi::Message reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    qi::serialization::serialize<P4>::write(request, p4);
    qi::serialization::serialize<P5>::write(request, p5);
    qi::serialization::serialize<P6>::write(request, p6);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(reply, ret);
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7) {
    qi::Message request;
    qi::Message reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    qi::serialization::serialize<P4>::write(request, p4);
    qi::serialization::serialize<P5>::write(request, p5);
    qi::serialization::serialize<P6>::write(request, p6);
    qi::serialization::serialize<P7>::write(request, p7);
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7) {
    qi::Message request;
    qi::Message reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    qi::serialization::serialize<P4>::write(request, p4);
    qi::serialization::serialize<P5>::write(request, p5);
    qi::serialization::serialize<P6>::write(request, p6);
    qi::serialization::serialize<P7>::write(request, p7);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(reply, ret);
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8) {
    qi::Message request;
    qi::Message reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    qi::serialization::serialize<P4>::write(request, p4);
    qi::serialization::serialize<P5>::write(request, p5);
    qi::serialization::serialize<P6>::write(request, p6);
    qi::serialization::serialize<P7>::write(request, p7);
    qi::serialization::serialize<P8>::write(request, p8);
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8) {
    qi::Message request;
    qi::Message reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request.writeString(signature);
    qi::serialization::serialize<P0>::write(request, p0);
    qi::serialization::serialize<P1>::write(request, p1);
    qi::serialization::serialize<P2>::write(request, p2);
    qi::serialization::serialize<P3>::write(request, p3);
    qi::serialization::serialize<P4>::write(request, p4);
    qi::serialization::serialize<P5>::write(request, p5);
    qi::serialization::serialize<P6>::write(request, p6);
    qi::serialization::serialize<P7>::write(request, p7);
    qi::serialization::serialize<P8>::write(request, p8);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(reply, ret);
    return ret;
  }

}
#endif  // _QIMESSAGING_CLIENT_HXX_
