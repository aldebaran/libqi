
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
    calldef.write<std::string>(hash);
    xCall(hash, calldef, resultdef);

    //TODO: optimise
    R ret;
    resultdef.read<R>(ret);
    return ret;
  }

  template <typename P0>
  void Client::callVoid(const std::string& methodName, const P0 &p0) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    void (*f)(const P0 &p0)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0>
  R Client::call(const std::string& methodName, const P0 &p0) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    R (*f)(const P0 &p0)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    xCall(hash, calldef, resultdef);

    //TODO: optimise
    R ret;
    resultdef.read<R>(ret);
    return ret;
  }

  template <typename P0, typename P1>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    void (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    calldef.write<P1>(p1);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    R (*f)(const P0 &p0, const P1 &p1)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    calldef.write<P1>(p1);
    xCall(hash, calldef, resultdef);

    //TODO: optimise
    R ret;
    resultdef.read<R>(ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    calldef.write<P1>(p1);
    calldef.write<P2>(p2);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    calldef.write<P1>(p1);
    calldef.write<P2>(p2);
    xCall(hash, calldef, resultdef);

    //TODO: optimise
    R ret;
    resultdef.read<R>(ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2, typename P3>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    calldef.write<P1>(p1);
    calldef.write<P2>(p2);
    calldef.write<P3>(p3);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2, typename P3>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    calldef.write<P1>(p1);
    calldef.write<P2>(p2);
    calldef.write<P3>(p3);
    xCall(hash, calldef, resultdef);

    //TODO: optimise
    R ret;
    resultdef.read<R>(ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2, typename P3, typename P4>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    calldef.write<P1>(p1);
    calldef.write<P2>(p2);
    calldef.write<P3>(p3);
    calldef.write<P4>(p4);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    calldef.write<P1>(p1);
    calldef.write<P2>(p2);
    calldef.write<P3>(p3);
    calldef.write<P4>(p4);
    xCall(hash, calldef, resultdef);

    //TODO: optimise
    R ret;
    resultdef.read<R>(ret);
    return ret;
  }

  template <typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  void Client::callVoid(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    void (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string hash = makeSignature(methodName, f);

    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    calldef.write<P1>(p1);
    calldef.write<P2>(p2);
    calldef.write<P3>(p3);
    calldef.write<P4>(p4);
    calldef.write<P5>(p5);
    xCall(hash, calldef, resultdef);
  }

  template <typename R, typename P0, typename P1, typename P2, typename P3, typename P4, typename P5>
  R Client::call(const std::string& methodName, const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
    qi::serialization::BinarySerializer calldef;
    qi::serialization::BinarySerializer resultdef;

    R (*f)(const P0 &p0, const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)  = 0;
    std::string hash = makeSignature(methodName, f);
    calldef.write<std::string>(hash);
    calldef.write<P0>(p0);
    calldef.write<P1>(p1);
    calldef.write<P2>(p2);
    calldef.write<P3>(p3);
    calldef.write<P4>(p4);
    calldef.write<P5>(p5);
    xCall(hash, calldef, resultdef);

    //TODO: optimise
    R ret;
    resultdef.read<R>(ret);
    return ret;
  }

}
#endif // __QI_MESSAGING_CLIENT_HXX_IN__
