///*
//** Author(s):
//**  - Cedric GESTES <gestes@aldebaran-robotics.com>
//**
//** Copyright (C) 2010 Aldebaran Robotics
//*/
//
//#include <alcommon-ng/messaging/server.hpp>
//#include <alcommon-ng/serialization/call_definition.hpp>
//#include <alcommon-ng/serialization/result_definition.hpp>
//#include <alcommon-ng/serialization/serialization.h>
//#include <alcommon-ng/transport/transport.hpp>
//
//using namespace AL::Serialization;
//
//namespace AL {
//  namespace Messaging {
//
//    template<typename T, typename R>
//    Server<T, R>::Server(const std::string &address)
//    {
//      _server = new AL::Transport::ZMQServer(address);
//      _server->setDataHandler(this);
//    }
//
//    template<typename T, typename R>
//    void Server<T, R>::onData(const std::string &data, std::string &result)
//    {
//      T def = Serializer::deserialize<T>(data);
//      boost::shared_ptr<R> res;
//
//      res = _onMessageDelegate->onMessage(def);
//      assert(res);
//      result = Serializer::serialize(*res);
//    }
//
//    template<typename T, typename R>
//    void Server<T, R>::run()
//    {
//      _server->run();
//    }
//
//  }
//}
