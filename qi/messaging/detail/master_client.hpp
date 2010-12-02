#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef _QI_MESSAGING_DETAIL_MASTER_CLIENT_HPP_
#define _QI_MESSAGING_DETAIL_MASTER_CLIENT_HPP_

#include <qi/messaging/detail/impl_base.hpp>
#include <qi/serialization.hpp>
#include <qi/transport/client.hpp>
#include <qi/transport/detail/network/master_endpoint.hpp>

namespace qi {
  namespace detail {
    class MasterClient: public ImplBase {
    public:
      MasterClient() {}
      MasterClient(const std::string name, const std::string& masterAddress) :
      _name(name),
        _masterAddress(masterAddress) {
          _endpointContext.name = _name;
          _endpointContext.contextID = _qiContext.getID();
      }
      virtual ~MasterClient() {}

      void init() {
        std::pair<std::string, int> masterEndpointAndPort;
        if (!qi::detail::validateMasterEndpoint(_masterAddress, masterEndpointAndPort)) {
          _isInitialized = false;
          qisError << "\"" << _name << "\" initialized with invalid master address: \""
            << _masterAddress << "\" All calls will fail." << std::endl;
          return;
        }
        _isInitialized = _transportClient.connect(masterEndpointAndPort.first);
        if (! _isInitialized ) {
          qisError << "\"" << _name << "\" could not connect to master "
            "at address \"" << masterEndpointAndPort.first << "\""
            << std::endl;
          return;
        }
      }

      const std::string& getName() const {
        return _name;
      }

      const std::string& getMasterAddress() const {
        return _masterAddress;
      }

      int getNewPort(const std::string& machineID) {
        if (!_isInitialized) {
          return 0;
        }
        static const std::string method("master.getNewPort::i:s");
        qi::transport::Buffer               ret;
        qi::serialization::Message msg;

        msg.writeString(method);
        msg.writeString(machineID);
        _transportClient.send(msg.str(), ret);
        qi::serialization::Message retSer(ret);
        int port;
        retSer.readInt(port);
        return port;
      }

      void registerMachine(const qi::detail::MachineContext& m) {
        if (!_isInitialized) {
          return;
        }
        static const std::string method = "master.registerMachine::v:sssi";
        qi::transport::Buffer               ret;
        qi::serialization::Message msg;
        msg.writeString(method);
        msg.writeString(m.machineID);
        msg.writeString(m.hostName);
        msg.writeString(m.publicIP);
        msg.writeInt(   m.platformID);
        _transportClient.send(msg.str(), ret);
      }

      void registerEndpoint(const qi::detail::EndpointContext& e) {
        if (!_isInitialized) {
          return;
        }
        static const std::string method("master.registerEndpoint::v:issssii");
        qi::transport::Buffer               ret;
        qi::serialization::Message msg;

        msg.writeString(method);
        msg.writeInt((int)e.type);
        msg.writeString(e.name);
        msg.writeString(e.endpointID);
        msg.writeString(e.contextID);
        msg.writeString(e.machineID);
        msg.writeInt(   e.processID);
        msg.writeInt(   e.port);
        _transportClient.send(msg.str(), ret);
      }

      void unregisterEndpoint(const qi::detail::EndpointContext& e) {
        if (!_isInitialized) {
          return;
        }
        static const std::string method("master.unregisterEndpoint::v:s");
        qi::transport::Buffer               ret;
        qi::serialization::Message msg;
        msg.writeString(method);
        msg.writeString(e.endpointID);
        _transportClient.send(msg.str(), ret);
      }

      std::string locateService(const std::string& methodSignature, const qi::detail::EndpointContext& e) {
        if (!_isInitialized) {
          return "";
        }
        qi::transport::Buffer               ret;
        qi::serialization::Message msg;
        static const std::string method("master.locateService::s:ss");
        msg.writeString(method);
        msg.writeString(methodSignature);
        msg.writeString(e.endpointID);
        _transportClient.send(msg.str(), ret);
        qi::serialization::Message retSer(ret);
        std::string endpoint;
        retSer.readString(endpoint);
        return endpoint;
      }

      void registerService(
        const std::string& methodSignature, const qi::detail::EndpointContext& e)
      {
        if (!_isInitialized) {
          return;
        }
        static const std::string method("master.registerService::v:ss");
        qi::transport::Buffer               ret;
        qi::serialization::Message msg;

        msg.writeString(method);
        msg.writeString(methodSignature);
        msg.writeString(e.endpointID);
        _transportClient.send(msg.str(), ret);
      }

      std::string locateTopic(const std::string& methodSignature, const qi::detail::EndpointContext& e) {
        if (!_isInitialized) {
          return "";
        }
        qi::transport::Buffer               ret;
        qi::serialization::Message msg;
        static const std::string method("master.locateTopic::s:ss");
        msg.writeString(method);
        msg.writeString(methodSignature);
        msg.writeString(e.endpointID);
        _transportClient.send(msg.str(), ret);
        qi::serialization::Message retSer(ret);
        std::string endpoint;
        retSer.readString(endpoint);
        return endpoint;
      }

      bool topicExists(const std::string& signature)
      {
        if (!_isInitialized) {
          return false;
        }
        static const std::string method("master.topicExists::b:s");
        qi::transport::Buffer               ret;
        qi::serialization::Message msg;

        msg.writeString(method);
        msg.writeString(signature);
        _transportClient.send(msg.str(), ret);
        qi::serialization::Message retSer(ret);
        bool exists;
        retSer.readBool(exists);
        return exists;
      }

      void registerTopic(
        const std::string& signature, const qi::detail::EndpointContext& e)
      {
        if (!_isInitialized) {
          return;
        }
        static const std::string method("master.registerTopic::v:ss");
        qi::transport::Buffer               ret;
        qi::serialization::Message msg;

        msg.writeString(method);
        msg.writeString(signature);
        msg.writeString(e.endpointID);
        _transportClient.send(msg.str(), ret);
      }

      std::string _name;
      std::string _masterAddress;

      /// <summary> The transport client used to talk with the master </summary>
      qi::transport::Client _transportClient;
    };
  }
}

#endif  // _QI_MESSAGING_DETAIL_MASTER_CLIENT_HPP_

