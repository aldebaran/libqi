
/*
* $autogen
*
*  Author(s):
*  - Chris Kilner  <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2012 Aldebaran Robotics
*/

#pragma once
#ifndef _QIMESSAGING_OBJECT_HXX_
#define _QIMESSAGING_OBJECT_HXX_

#include <qimessaging/buffer.hpp>

namespace qi {


  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R>
  R Object::call(const std::string& method) {
  qi::Buffer    bufReq;
  qi::Buffer    bufRep;
  qi::DataStream request(&bufReq);
  qi::DataStream reply(&bufRep);

    R (*f)()  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    metaCall(method, signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply >> ret;
    return ret;
  }

  /// <summary> Calls a void method </summary>
  inline void Object::callVoid(const std::string& method) {
    qi::Buffer    bufReq;
    qi::Buffer    bufRep;
    qi::DataStream request(&bufReq);
    qi::DataStream reply(&bufRep);

    void (*f)()  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    metaCall(method, signature, request, reply);
  }

  /// <summary> Calls a void method </summary>
  template <typename P0>
  void Object::callVoid(const std::string& method, const P0 &p0) {
    qi::Buffer    bufReq;
    qi::Buffer    bufRep;
    qi::DataStream request(&bufReq);
    qi::DataStream reply(&bufRep);

    void (*f)(const P0 &p0)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    metaCall(method, signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0>
  R Object::call(const std::string& method, const P0 &p0) {
  qi::Buffer    bufReq;
  qi::Buffer    bufRep;
  qi::DataStream request(&bufReq);
  qi::DataStream reply(&bufRep);

    R (*f)(const P0 &p0)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    metaCall(method, signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1>
  void Object::callVoid(const std::string& method, const P0 &p0, const P1 &p1) {
    qi::Buffer    bufReq;
    qi::Buffer    bufRep;
    qi::DataStream request(&bufReq);
    qi::DataStream reply(&bufRep);

    void (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    metaCall(method, signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1>
  R Object::call(const std::string& method, const P0 &p0, const P1 &p1) {
  qi::Buffer    bufReq;
  qi::Buffer    bufRep;
  qi::DataStream request(&bufReq);
  qi::DataStream reply(&bufRep);

    R (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    metaCall(method, signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2>
  void Object::callVoid(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::Buffer    bufReq;
    qi::Buffer    bufRep;
    qi::DataStream request(&bufReq);
    qi::DataStream reply(&bufRep);

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    metaCall(method, signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2>
  R Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2) {
  qi::Buffer    bufReq;
  qi::Buffer    bufRep;
  qi::DataStream request(&bufReq);
  qi::DataStream reply(&bufRep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    metaCall(method, signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3>
  void Object::callVoid(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::Buffer    bufReq;
    qi::Buffer    bufRep;
    qi::DataStream request(&bufReq);
    qi::DataStream reply(&bufRep);

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    metaCall(method, signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3>
  R Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
  qi::Buffer    bufReq;
  qi::Buffer    bufRep;
  qi::DataStream request(&bufReq);
  qi::DataStream reply(&bufRep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    metaCall(method, signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4>
  void Object::callVoid(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::Buffer    bufReq;
    qi::Buffer    bufRep;
    qi::DataStream request(&bufReq);
    qi::DataStream reply(&bufRep);

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    metaCall(method, signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4>
  R Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
  qi::Buffer    bufReq;
  qi::Buffer    bufRep;
  qi::DataStream request(&bufReq);
  qi::DataStream reply(&bufRep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    metaCall(method, signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  void Object::callVoid(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::Buffer    bufReq;
    qi::Buffer    bufRep;
    qi::DataStream request(&bufReq);
    qi::DataStream reply(&bufRep);

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    metaCall(method, signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  R Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
  qi::Buffer    bufReq;
  qi::Buffer    bufRep;
  qi::DataStream request(&bufReq);
  qi::DataStream reply(&bufRep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    metaCall(method, signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  void Object::callVoid(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) {
    qi::Buffer    bufReq;
    qi::Buffer    bufRep;
    qi::DataStream request(&bufReq);
    qi::DataStream reply(&bufRep);

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    metaCall(method, signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  R Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) {
  qi::Buffer    bufReq;
  qi::Buffer    bufRep;
  qi::DataStream request(&bufReq);
  qi::DataStream reply(&bufRep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    metaCall(method, signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
  void Object::callVoid(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7) {
    qi::Buffer    bufReq;
    qi::Buffer    bufRep;
    qi::DataStream request(&bufReq);
    qi::DataStream reply(&bufRep);

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    request << p7;
    metaCall(method, signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
  R Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7) {
  qi::Buffer    bufReq;
  qi::Buffer    bufRep;
  qi::DataStream request(&bufReq);
  qi::DataStream reply(&bufRep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    request << p7;
    metaCall(method, signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply >> ret;
    return ret;
  }


  /// <summary> Calls a void method </summary>
  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
  void Object::callVoid(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8) {
    qi::Buffer    bufReq;
    qi::Buffer    bufRep;
    qi::DataStream request(&bufReq);
    qi::DataStream reply(&bufRep);

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    request << p7;
    request << p8;
    metaCall(method, signature, request, reply);
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
  R Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8) {
  qi::Buffer    bufReq;
  qi::Buffer    bufRep;
  qi::DataStream request(&bufReq);
  qi::DataStream reply(&bufRep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request << p0;
    request << p1;
    request << p2;
    request << p3;
    request << p4;
    request << p5;
    request << p6;
    request << p7;
    request << p8;
    metaCall(method, signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply >> ret;
    return ret;
  }

}
#endif  // _QIMESSAGING_OBJECT_HXX_
