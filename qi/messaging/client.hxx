
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
#ifndef   __QI_MESSAGING_CLIENT_HXX_IN__
#define   __QI_MESSAGING_CLIENT_HXX_IN__

#include <qi/serialization/serializer.hpp>

namespace qi {


  template <typename R>
  R Client::call(const std::string& methodName) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    void (*f)(const P0 &p0)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0>
  R Client::call(const std::string& methodName, const P0 &p0) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    void (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.writeString(hash);
    qi::serialization::serialize<P0>::write(calldef, p0);
    qi::serialization::serialize<P1>::write(calldef, p1);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

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

}
#endif // __QI_MESSAGING_CLIENT_HXX_IN__
