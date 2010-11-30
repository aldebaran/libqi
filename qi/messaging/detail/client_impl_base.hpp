#pragma once
/*
** Author(s):
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/
#ifndef   __QI_MESSAGING_DETAIL_CLIENT_IMPL_BASE_HPP__
#define   __QI_MESSAGING_DETAIL_CLIENT_IMPL_BASE_HPP__

#include <qi/messaging/detail/impl_base.hpp>
#include <qi/transport/client.hpp>
#include <qi/serialization.hpp>

namespace qi {
  namespace detail {
    class ClientImplBase: public ImplBase {
    public:
      ClientImplBase() {}
      virtual ~ClientImplBase() {}

    protected:

      // ** Helper methods for talking to the master **

      int xGetNewPortFromMaster(const std::string& machineID) {
        if (!_isInitialized) {
          return 0;
        }
        static const std::string method("master.getNewPort::i:s");
        qi::transport::Buffer               ret;
        qi::serialization::BinarySerializer msg;

        msg.writeString(method);
        msg.writeString(machineID);
        _transportClient.send(msg.str(), ret);
        qi::serialization::BinarySerializer retSer(ret);
        int port;
        retSer.readInt(port);
        return port;
      }

      void xRegisterMachineWithMaster() {
        if (!_isInitialized) {
          return;
        }
        static const std::string method = "master.registerMachine::v:sssi";
        qi::transport::Buffer               ret;
        qi::serialization::BinarySerializer msg;
        msg.writeString(method);
        msg.writeString(_machineContext.machineID);
        msg.writeString(_machineContext.hostName);
        msg.writeString(_machineContext.publicIP);
        msg.writeInt(   _machineContext.platformID);
        _transportClient.send(msg.str(), ret);
      }

      void xRegisterSelfWithMaster() {
        if (!_isInitialized) {
          return;
        }
        static const std::string method("master.registerEndpoint::v:issssii");
        qi::transport::Buffer               ret;
        qi::serialization::BinarySerializer msg;

        msg.writeString(method);
        msg.writeInt((int)_endpointContext.type);
        msg.writeString(_endpointContext.name);
        msg.writeString(_endpointContext.endpointID);
        msg.writeString(_endpointContext.contextID);
        msg.writeString(_endpointContext.machineID);
        msg.writeInt(   _endpointContext.processID);
        msg.writeInt(   _endpointContext.port);
        _transportClient.send(msg.str(), ret);
      }

      void xUnregisterSelfWithMaster() {
        if (!_isInitialized) {
          return;
        }
        static const std::string method("master.unregisterEndpoint::v:s");
        qi::transport::Buffer               ret;
        qi::serialization::BinarySerializer msg;
        msg.writeString(method);
        msg.writeString(_endpointContext.endpointID);
        _transportClient.send(msg.str(), ret);
      }

      std::string xLocateServiceWithMaster(const std::string& methodSignature) {
        if (!_isInitialized) {
          return "";
        }
        qi::transport::Buffer               ret;
        qi::serialization::BinarySerializer msg;
        static const std::string method("master.locateService::s:ss");
        msg.writeString(method);
        msg.writeString(methodSignature);
        msg.writeString(_endpointContext.endpointID);
        _transportClient.send(msg.str(), ret);
        qi::serialization::BinarySerializer retSer(ret);
        std::string endpoint;
        retSer.readString(endpoint);
        return endpoint;
      }

      /// <summary> The transport client used to talk with the master </summary>
      qi::transport::Client _transportClient;
    };
  }
}

#endif // __QI_MESSAGING_DETAIL_CLIENT_IMPL_BASE_HPP__

