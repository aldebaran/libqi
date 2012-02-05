
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


namespace qi {


  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R>
  R Client::call(const std::string& methodName) {
    qi::DataStream request;
    qi::DataStream reply;

    R (*f)()  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    request >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0>
  void Client::callVoid(const std::string& methodName, const P0 &p0) {
    qi::DataStream request;
    qi::DataStream reply;

    void (*f)(const P0 &p0)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0>
  R Client::call(const std::string& methodName, const P0 &p0) {
    qi::DataStream request;
    qi::DataStream reply;

    R (*f)(const P0 &p0)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    request >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1) {
    qi::DataStream request;
    qi::DataStream reply;

    void (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1) {
    qi::DataStream request;
    qi::DataStream reply;

    R (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    request >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::DataStream request;
    qi::DataStream reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::DataStream request;
    qi::DataStream reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    request >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::DataStream request;
    qi::DataStream reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::DataStream request;
    qi::DataStream reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    request >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::DataStream request;
    qi::DataStream reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::DataStream request;
    qi::DataStream reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    request >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::DataStream request;
    qi::DataStream reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::DataStream request;
    qi::DataStream reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    request >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) {
    qi::DataStream request;
    qi::DataStream reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) {
    qi::DataStream request;
    qi::DataStream reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    request >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7) {
    qi::DataStream request;
    qi::DataStream reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    request << p7;
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7) {
    qi::DataStream request;
    qi::DataStream reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    request << p7;
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    request >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8) {
    qi::DataStream request;
    qi::DataStream reply;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    request << p7;
    request << p8;
    xCall(signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8) {
    qi::DataStream request;
    qi::DataStream reply;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8)  = 0;
    std::string signature = makeFunctionSignature(methodName, f);
    request << (signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    request << p7;
    request << p8;
    xCall(signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    request >> ret;
    return ret;
  }

}
#endif  // _QIMESSAGING_CLIENT_HXX_
