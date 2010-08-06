///*
//** Author(s):
//**  - Cedric GESTES <gestes@aldebaran-robotics.com>
//**
//** Copyright (C) 2010 Aldebaran Robotics
//*/
//
//#include <alcommon-ng/messaging/client.hpp>
//#include <alcommon-ng/transport/zeromq/zmqclient.hpp>
////#include <alcommon-ng/serialization/call_definition.hpp>
////#include <alcommon-ng/serialization/result_definition.hpp>
////#include <alcommon-ng/serialization/serialization.h>
//
//using namespace AL::Serialization;
//
//namespace AL {
//  namespace Messaging {
//
//    template<typename T, typename R>
//    Client<T, R>::Client(const std::string &address)
//    {
//      _client = new AL::Transport::ZMQClient(address);
//    }
//
//    //boost::shared_ptr<R> Client::send(T &def)
//    //{
//    //  std::string tosend = Serializer::serialize(def);
//    //  std::string torecv;
//
//    //  _client->send(tosend, torecv);
//    //  return Serializer::deserializeToPtr<R>(torecv);
//    //}
//
//  }
//}
