#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_MESSAGING_CLIENT_HXX_
#define _QI_MESSAGING_CLIENT_HXX_

#include <qi/serialization/serializer.hpp>

namespace qi {


  template <typename R>
  R Client::call(const std::string& methodName) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    R (*f)()  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.writeString(hash);
    xCall(hash, calldef, resultdef);

    //Optimise? I think compiler is smart enought to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(resultdef, ret);
    return ret;
  }

  template <typename P0>
  void Client::callVoid(const std::string& methodName, const P0 &p0) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    void (*f)(const P0 &p0)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0>
  R Client::call(const std::string& methodName, const P0 &p0) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    R (*f)(const P0 &p0)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    xCall(hash, calldef, resultdef);

    //Optimise? I think compiler is smart enought to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(resultdef, ret);
    return ret;
  }

  template <typename P0, typename P1>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    void (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    R (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    xCall(hash, calldef, resultdef);

    //Optimise? I think compiler is smart enought to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(resultdef, ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    xCall(hash, calldef, resultdef);

    //Optimise? I think compiler is smart enought to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(resultdef, ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2, typename P3>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2, typename P3>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    xCall(hash, calldef, resultdef);

    //Optimise? I think compiler is smart enought to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(resultdef, ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2, typename P3, typename P4>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    qi::serialization::serialize<P4>::write(calldef, p4);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    qi::serialization::serialize<P4>::write(calldef, p4);
    xCall(hash, calldef, resultdef);

    //Optimise? I think compiler is smart enought to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(resultdef, ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    qi::serialization::serialize<P4>::write(calldef, p4);
    qi::serialization::serialize<P5>::write(calldef, p5);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    qi::serialization::serialize<P4>::write(calldef, p4);
    qi::serialization::serialize<P5>::write(calldef, p5);
    xCall(hash, calldef, resultdef);

    //Optimise? I think compiler is smart enought to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(resultdef, ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    qi::serialization::serialize<P4>::write(calldef, p4);
    qi::serialization::serialize<P5>::write(calldef, p5);
    qi::serialization::serialize<P6>::write(calldef, p6);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    qi::serialization::serialize<P4>::write(calldef, p4);
    qi::serialization::serialize<P5>::write(calldef, p5);
    qi::serialization::serialize<P6>::write(calldef, p6);
    xCall(hash, calldef, resultdef);

    //Optimise? I think compiler is smart enought to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(resultdef, ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    qi::serialization::serialize<P4>::write(calldef, p4);
    qi::serialization::serialize<P5>::write(calldef, p5);
    qi::serialization::serialize<P6>::write(calldef, p6);
    qi::serialization::serialize<P7>::write(calldef, p7);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    qi::serialization::serialize<P4>::write(calldef, p4);
    qi::serialization::serialize<P5>::write(calldef, p5);
    qi::serialization::serialize<P6>::write(calldef, p6);
    qi::serialization::serialize<P7>::write(calldef, p7);
    xCall(hash, calldef, resultdef);

    //Optimise? I think compiler is smart enought to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(resultdef, ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    qi::serialization::serialize<P4>::write(calldef, p4);
    qi::serialization::serialize<P5>::write(calldef, p5);
    qi::serialization::serialize<P6>::write(calldef, p6);
    qi::serialization::serialize<P7>::write(calldef, p7);
    qi::serialization::serialize<P8>::write(calldef, p8);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8) {
    qi::serialization::Message calldef;
    qi::serialization::Message resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5, const P6 &p6, const P7 &p7, const P8 &p8)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    qi::serialization::serialize<P2>::write(calldef, p2);
    qi::serialization::serialize<P3>::write(calldef, p3);
    qi::serialization::serialize<P4>::write(calldef, p4);
    qi::serialization::serialize<P5>::write(calldef, p5);
    qi::serialization::serialize<P6>::write(calldef, p6);
    qi::serialization::serialize<P7>::write(calldef, p7);
    qi::serialization::serialize<P8>::write(calldef, p8);
    xCall(hash, calldef, resultdef);

    //Optimise? I think compiler is smart enought to inline the returned object
    R ret;
    qi::serialization::serialize<R>::read(resultdef, ret);
    return ret;
  }

}
#endif  // _QI_MESSAGING_CLIENT_HXX_
