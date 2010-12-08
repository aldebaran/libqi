/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qi/messaging/src/master_client.hpp>
#include <qi/serialization.hpp>
#include <qi/messaging/src/network/master_endpoint.hpp>

namespace qi {
  namespace detail {

    using qi::transport::Buffer;
    using qi::serialization::Message;

    MasterClient::~MasterClient() {}

    MasterClient::MasterClient(const std::string name,
                               Context *ctx)
    {
      _endpointContext.name = name;
      _endpointContext.contextID = _qiContext.getID();
    }

//TODO:    _masterAddress(masterAddress)

    void MasterClient::init() {
      std::pair<std::string, int> masterEndpointAndPort;
      if (!qi::detail::validateMasterEndpoint(_masterAddress, masterEndpointAndPort)) {
        _isInitialized = false;
        qisError << "\"" << _endpointContext.name << "\" initialized with invalid master address: \""
            << _masterAddress << "\" All calls will fail." << std::endl;
        return;
      }
      _isInitialized = _transportClient.connect(masterEndpointAndPort.first);
      if (! _isInitialized ) {
        qisError << "\"" << _endpointContext.name << "\" could not connect to master "
            "at address \"" << masterEndpointAndPort.first << "\""
            << std::endl;
        return;
      }
    }

    const std::string& MasterClient::getMasterAddress() const {
      return _masterAddress;
    }

    int MasterClient::getNewPort(const std::string& machineID) {
      if (!_isInitialized) {
        return 0;
      }
      static const std::string method("master.getNewPort::i:s");
      Buffer ret;
      Message msg;

      msg.writeString(method);
      msg.writeString(machineID);
      _transportClient.send(msg.str(), ret);
      Message retSer(ret);
      int port;
      retSer.readInt(port);
      return port;
    }

    void MasterClient::registerMachine(const qi::detail::MachineContext& m) {
      if (!_isInitialized) {
        return;
      }
      static const std::string method = "master.registerMachine::v:sssi";
      Buffer ret;
      Message msg;
      msg.writeString(method);
      msg.writeString(m.hostName);
      msg.writeString(m.machineID);
      msg.writeString(m.publicIP);
      msg.writeInt(   m.platformID);
      _transportClient.send(msg.str(), ret);
    }

    void MasterClient::registerEndpoint(const qi::detail::EndpointContext& e) {
      if (!_isInitialized) {
        return;
      }
      static const std::string method("master.registerEndpoint::v:issssii");
      Buffer ret;
      Message msg;

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

    void MasterClient::unregisterEndpoint(const qi::detail::EndpointContext& e) {
      if (!_isInitialized) {
        return;
      }
      static const std::string method("master.unregisterEndpoint::v:s");
      Buffer ret;
      Message msg;
      msg.writeString(method);
      msg.writeString(e.endpointID);
      _transportClient.send(msg.str(), ret);
    }

    std::string MasterClient::locateService(const std::string& methodSignature, const qi::detail::EndpointContext& e) {
      if (!_isInitialized) {
        return "";
      }
      Buffer ret;
      Message msg;
      static const std::string method("master.locateService::s:ss");
      msg.writeString(method);
      msg.writeString(methodSignature);
      msg.writeString(e.endpointID);
      _transportClient.send(msg.str(), ret);
      Message retSer(ret);
      std::string endpoint;
      retSer.readString(endpoint);
      return endpoint;
    }

    void MasterClient::registerService(
        const std::string& methodSignature, const qi::detail::EndpointContext& e)
    {
      if (!_isInitialized) {
        return;
      }
      static const std::string method("master.registerService::v:ss");
      Buffer ret;
      Message msg;

      msg.writeString(method);
      msg.writeString(methodSignature);
      msg.writeString(e.endpointID);
      _transportClient.send(msg.str(), ret);
    }

    void MasterClient::unregisterService(const std::string& methodSignature)
    {
      if (!_isInitialized) {
        return;
      }
      static const std::string method("master.unregisterService::v:s");
      Buffer ret;
      Message msg;

      msg.writeString(method);
      msg.writeString(methodSignature);
      _transportClient.send(msg.str(), ret);
    }

    std::string MasterClient::locateTopic(const std::string& methodSignature, const qi::detail::EndpointContext& e) {
      if (!_isInitialized) {
        return "";
      }
      Buffer ret;
      Message msg;
      static const std::string method("master.locateTopic::s:ss");
      msg.writeString(method);
      msg.writeString(methodSignature);
      msg.writeString(e.endpointID);
      _transportClient.send(msg.str(), ret);
      Message retSer(ret);
      std::string endpoint;
      retSer.readString(endpoint);
      return endpoint;
    }

    bool MasterClient::topicExists(const std::string& signature)
    {
      if (!_isInitialized) {
        return false;
      }
      static const std::string method("master.topicExists::b:s");
      Buffer ret;
      Message msg;

      msg.writeString(method);
      msg.writeString(signature);
      _transportClient.send(msg.str(), ret);
      Message retSer(ret);
      bool exists;
      retSer.readBool(exists);
      return exists;
    }

    void MasterClient::registerTopic(
        const std::string& signature, const qi::detail::EndpointContext& e)
    {
      if (!_isInitialized) {
        return;
      }
      static const std::string method("master.registerTopic::v:ss");
      Buffer ret;
      Message msg;

      msg.writeString(method);
      msg.writeString(signature);
      msg.writeString(e.endpointID);
      _transportClient.send(msg.str(), ret);
    }

    void MasterClient::unregisterTopic(
      const std::string& signature,
      const qi::detail::EndpointContext& e)
    {
      if (!_isInitialized) {
        return;
      }
      static const std::string method("master.unregisterTopic::v:ss");
      Buffer ret;
      Message msg;

      msg.writeString(method);
      msg.writeString(signature);
      msg.writeString(e.endpointID);
      _transportClient.send(msg.str(), ret);
    }
  }
}


