/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include "src/messaging/master_client.hpp"
#include <qi/serialization.hpp>
#include "src/messaging/network/master_endpoint.hpp"

namespace qi {
  namespace detail {

    using qi::transport::Buffer;
    using qi::serialization::Message;

    static const std::string methodUnregisterEndpoint("master.unregisterEndpoint::v:s");
    static const std::string methodRegisterTopic("master.registerTopic::v:sbs");
    static const std::string methodUnregisterTopic("master.unregisterTopic::v:ss");
    static const std::string methodGetNewPort("master.getNewPort::i:s");
    static const std::string methodRegisterMachine("master.registerMachine::v:sssi");
    static const std::string methodRegisterEndpoint("master.registerEndpoint::v:issssii");
    static const std::string methodTopicExists("master.topicExists::b:s");
    static const std::string methodLocateService("master.locateService::s:ss");
    static const std::string methodRegisterService("master.registerService::v:ss");
    static const std::string methodUnregisterService("master.unregisterService::v:s");
    static const std::string methodLocateTopic("master.locateTopic::s:ss");
    static const std::string methodRegisterTopicParticipant("master.registerTopicParticipant::v:ss");

    MasterClient::~MasterClient() {
    }

    MasterClient::MasterClient(qi::Context *ctx)
      : _isInitialized(false),
        _qiContextPtr( (ctx == NULL)? getDefaultQiContextPtr() : ctx),
        _transportClient(_qiContextPtr->getTransportContext())
    {
    }

    void MasterClient::connect(const std::string masterAddress) {
      _masterAddress = masterAddress;
      std::pair<std::string, int> masterEndpointAndPort;
      if (!qi::detail::validateMasterEndpoint(_masterAddress, masterEndpointAndPort)) {
        _isInitialized = false;
        qisError << "Initialized with invalid master address: \""
            << _masterAddress << "\" All calls will fail." << std::endl;
        return;
      }
      _isInitialized = _transportClient.connect(masterEndpointAndPort.first);
      if (! _isInitialized ) {
        qisError << "Could not connect to master "
            "at address \"" << masterEndpointAndPort.first << "\""
            << std::endl;
        return;
      }
    }

    bool MasterClient::isInitialized() const {
      return _isInitialized;
    }

    const std::string& MasterClient::getMasterAddress() const {
      return _masterAddress;
    }

    int MasterClient::getNewPort(const std::string& machineID) {
      if (!_isInitialized) {
        return 0;
      }
      Buffer ret;
      Message msg;

      msg.writeString(methodGetNewPort);
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
      Buffer ret;
      Message msg;
      msg.writeString(methodRegisterMachine);
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
      Buffer ret;
      Message msg;

      msg.writeString(methodRegisterEndpoint);
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
      Buffer ret;
      Message msg;
      msg.writeString(methodUnregisterEndpoint);
      msg.writeString(e.endpointID);
      _transportClient.send(msg.str(), ret);
    }

    std::string MasterClient::locateService(const std::string& methodSignature, const qi::detail::EndpointContext& e) {
      if (!_isInitialized) {
        return "";
      }
      Buffer ret;
      Message msg;
      msg.writeString(methodLocateService);
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
      Buffer ret;
      Message msg;

      msg.writeString(methodRegisterService);
      msg.writeString(methodSignature);
      msg.writeString(e.endpointID);
      _transportClient.send(msg.str(), ret);
    }

    void MasterClient::unregisterService(const std::string& methodSignature)
    {
      if (!_isInitialized) {
        return;
      }
      Buffer ret;
      Message msg;

      msg.writeString(methodUnregisterService);
      msg.writeString(methodSignature);
      _transportClient.send(msg.str(), ret);
    }

    std::string MasterClient::locateTopic(const std::string& methodSignature, const qi::detail::EndpointContext& e) {
      if (!_isInitialized) {
        return "";
      }
      Buffer ret;
      Message msg;

      msg.writeString(methodLocateTopic);
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
      Buffer ret;
      Message msg;

      msg.writeString(methodTopicExists);
      msg.writeString(signature);
      _transportClient.send(msg.str(), ret);
      Message retSer(ret);
      bool exists;
      retSer.readBool(exists);
      return exists;
    }

    void MasterClient::registerTopic(
        const std::string& signature, const bool& isManyToMany,
        const qi::detail::EndpointContext& e)
    {
      if (!_isInitialized) {
        return;
      }
      Buffer ret;
      Message msg;

      msg.writeString(methodRegisterTopic);
      msg.writeString(signature);
      msg.writeBool(isManyToMany);
      msg.writeString(e.endpointID);
      _transportClient.send(msg.str(), ret);
    }

    void MasterClient::registerTopicParticipant(
      const std::string& signature,
      const std::string& endpointID)
    {
      if (!_isInitialized) {
        return;
      }
      Buffer ret;
      Message msg;

      msg.writeString(methodRegisterTopicParticipant);
      msg.writeString(signature);
      msg.writeString(endpointID);
      _transportClient.send(msg.str(), ret);
    }

    void MasterClient::unregisterTopic(
      const std::string& signature,
      const qi::detail::EndpointContext& e)
    {
      if (!_isInitialized) {
        return;
      }
      Buffer ret;
      Message msg;

      msg.writeString(methodUnregisterTopic);
      msg.writeString(signature);
      msg.writeString(e.endpointID);
      _transportClient.send(msg.str(), ret);
    }
  }
}


