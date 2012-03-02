
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
#include <qimessaging/future.hpp>

namespace qi {


  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R>
  qi::Future<R> Object::call(const std::string& method) {
    qi::Promise<R>        promise;
    qi::Buffer            breq;
    qi::Buffer            brep;
    qi::FunctorParameters request(&breq);
    qi::FunctorResult     reply(&brep);

    R (*f)()  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    metaCall(metaObject()._methodsNameToIdx[method], signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply.datastream() >> ret;
    return promise.future();
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0>
  qi::Future<R> Object::call(const std::string& method, const P0 &p0) {
    qi::Promise<R>        promise;
    qi::Buffer            breq;
    qi::Buffer            brep;
    qi::FunctorParameters request(&breq);
    qi::FunctorResult     reply(&brep);

    R (*f)(const P0 &p0)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request.datastream() << p0;
    metaCall(metaObject()._methodsNameToIdx[method], signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply.datastream() >> ret;
    return promise.future();
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1>
  qi::Future<R> Object::call(const std::string& method, const P0 &p0, const P1 &p1) {
    qi::Promise<R>        promise;
    qi::Buffer            breq;
    qi::Buffer            brep;
    qi::FunctorParameters request(&breq);
    qi::FunctorResult     reply(&brep);

    R (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request.datastream() << p0;
    request.datastream() << p1;
    metaCall(metaObject()._methodsNameToIdx[method], signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply.datastream() >> ret;
    return promise.future();
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2>
  qi::Future<R> Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::Promise<R>        promise;
    qi::Buffer            breq;
    qi::Buffer            brep;
    qi::FunctorParameters request(&breq);
    qi::FunctorResult     reply(&brep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request.datastream() << p0;
    request.datastream() << p1;
    request.datastream() << p2;
    metaCall(metaObject()._methodsNameToIdx[method], signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply.datastream() >> ret;
    return promise.future();
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3>
  qi::Future<R> Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::Promise<R>        promise;
    qi::Buffer            breq;
    qi::Buffer            brep;
    qi::FunctorParameters request(&breq);
    qi::FunctorResult     reply(&brep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request.datastream() << p0;
    request.datastream() << p1;
    request.datastream() << p2;
    request.datastream() << p3;
    metaCall(metaObject()._methodsNameToIdx[method], signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply.datastream() >> ret;
    return promise.future();
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4>
  qi::Future<R> Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::Promise<R>        promise;
    qi::Buffer            breq;
    qi::Buffer            brep;
    qi::FunctorParameters request(&breq);
    qi::FunctorResult     reply(&brep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request.datastream() << p0;
    request.datastream() << p1;
    request.datastream() << p2;
    request.datastream() << p3;
    request.datastream() << p4;
    metaCall(metaObject()._methodsNameToIdx[method], signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply.datastream() >> ret;
    return promise.future();
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  qi::Future<R> Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::Promise<R>        promise;
    qi::Buffer            breq;
    qi::Buffer            brep;
    qi::FunctorParameters request(&breq);
    qi::FunctorResult     reply(&brep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request.datastream() << p0;
    request.datastream() << p1;
    request.datastream() << p2;
    request.datastream() << p3;
    request.datastream() << p4;
    request.datastream() << p5;
    metaCall(metaObject()._methodsNameToIdx[method], signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply.datastream() >> ret;
    return promise.future();
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  qi::Future<R> Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) {
    qi::Promise<R>        promise;
    qi::Buffer            breq;
    qi::Buffer            brep;
    qi::FunctorParameters request(&breq);
    qi::FunctorResult     reply(&brep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request.datastream() << p0;
    request.datastream() << p1;
    request.datastream() << p2;
    request.datastream() << p3;
    request.datastream() << p4;
    request.datastream() << p5;
    request.datastream() << p6;
    metaCall(metaObject()._methodsNameToIdx[method], signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply.datastream() >> ret;
    return promise.future();
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
  qi::Future<R> Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7) {
    qi::Promise<R>        promise;
    qi::Buffer            breq;
    qi::Buffer            brep;
    qi::FunctorParameters request(&breq);
    qi::FunctorResult     reply(&brep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request.datastream() << p0;
    request.datastream() << p1;
    request.datastream() << p2;
    request.datastream() << p3;
    request.datastream() << p4;
    request.datastream() << p5;
    request.datastream() << p6;
    request.datastream() << p7;
    metaCall(metaObject()._methodsNameToIdx[method], signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply.datastream() >> ret;
    return promise.future();
  }

  /// <summary> Calls a method </summary>
  /// <returns> The response </returns>
  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
  qi::Future<R> Object::call(const std::string& method, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8) {
    qi::Promise<R>        promise;
    qi::Buffer            breq;
    qi::Buffer            brep;
    qi::FunctorParameters request(&breq);
    qi::FunctorResult     reply(&brep);

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8)  = 0;
    std::string signature(method);
    signature += "::";
    signatureFromObject::value(f, signature);
    request.datastream() << p0;
    request.datastream() << p1;
    request.datastream() << p2;
    request.datastream() << p3;
    request.datastream() << p4;
    request.datastream() << p5;
    request.datastream() << p6;
    request.datastream() << p7;
    request.datastream() << p8;
    metaCall(metaObject()._methodsNameToIdx[method], signature, request, reply);

    // Optimise? I think compiler is smart enough to inline the returned object
    R ret;
    reply.datastream() >> ret;
    return promise.future();
  }

}
#endif  // _QIMESSAGING_OBJECT_HXX_
