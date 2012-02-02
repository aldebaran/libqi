/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include "src/messaging/network/ip_address.hpp"
#include "src/messaging/master_client.hpp"
#include <qimessaging/serialization.hpp>
#include <qimessaging/transport/buffer.hpp>
#include "src/messaging/network/master_endpoint.hpp"


namespace qi {
  namespace detail {

    static const std::string methodUnregisterEndpoint("master.unregisterEndpoint::v(s)");
    static const std::string methodRegisterTopic("master.registerTopic::v(sbs)");
    static const std::string methodUnregisterTopic("master.unregisterTopic::v(ss)");
    static const std::string methodGetNewPort("master.getNewPort::i(s)");
    static const std::string methodRegisterMachine("master.registerMachine::v(sssi)");
    static const std::string methodRegisterEndpoint("master.registerEndpoint::v(issssii)");
    static const std::string methodTopicExists("master.topicExists::b(s)");
    static const std::string methodLocateService("master.locateService::s(ss)");
    static const std::string methodRegisterService("master.registerService::v(ss)");
    static const std::string methodUnregisterService("master.unregisterService::v(s)");
    static const std::string methodLocateTopic("master.locateTopic::s(ss)");
    static const std::string methodRegisterTopicParticipant("master.registerTopicParticipant::v(ss)");

    MasterClient::~MasterClient() {
    }

    MasterClient::MasterClient(qi::Context *ctx)
      : _isInitialized(false),
        _qiContextPtr( (ctx == NULL)? getDefaultQiContextPtr() : ctx),
        _transportClient()
    {
    }

    void MasterClient::connect(const std::string masterAddress)
    {
      _masterAddress = masterAddress;
      std::pair<std::string, int> masterEndpointAndPort;
      if (!qi::detail::validateMasterEndpoint(_masterAddress, masterEndpointAndPort))
      {
        _isInitialized = false;
        qiLogError("qimessaging") << "Initialized with invalid master address: \""
            << _masterAddress << "\" All calls will fail." << std::endl;
        return;
      }

      //CTAF: todo
      //_isInitialized = _transportClient.connect(masterEndpointAndPort.first);
      if (! _isInitialized ) {
        qiLogError("qimessaging") << "Could not connect to master "
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
      qi::transport::Buffer ret;
      qi::DataStream msg;

      msg << methodGetNewPort;
      msg << machineID;

      //CTAF:todo
      //_transportClient.send(msg.str(), ret);
      qi::DataStream retSer(ret);
      int port;
      retSer >> port;
      return port;
    }

    void MasterClient::registerMachine(const qi::detail::MachineContext& m) {
      if (!_isInitialized) {
        return;
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;
      msg << methodRegisterMachine;
      msg << m.hostName;
      msg << m.machineID;
      msg << m.publicIP;
      msg << m.platformID;
      //CTAF:todo
      //_transportClient.send(msg.str(), ret);
    }

    void MasterClient::registerEndpoint(const qi::detail::EndpointContext& e) {
      if (!_isInitialized) {
        return;
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;

      msg << (methodRegisterEndpoint);
      msg << ((int)e.type);
      msg << (e.name);
      msg << (e.endpointID);
      msg << (e.contextID);
      msg << (e.machineID);
      msg << (e.processID);
      msg << (e.port);
      //CTAF:todo

      //_transportClient.send(msg.str(), ret);
    }

    void MasterClient::unregisterEndpoint(const qi::detail::EndpointContext& e) {
      if (!_isInitialized) {
        return;
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;
      msg << (methodUnregisterEndpoint);
      msg << (e.endpointID);
      //CTAF:todo
      //_transportClient.send(msg.str(), ret);
    }

    std::string MasterClient::locateService(const std::string& methodSignature, const qi::detail::EndpointContext& e) {
      if (!_isInitialized) {
        return "";
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;
      msg << (methodLocateService);
      msg << (methodSignature);
      msg << (e.endpointID);
      //CTAF:todo
      //_transportClient.send(msg.str(), ret);
      qi::DataStream retSer(ret);
      std::string endpoint;
      retSer >> (endpoint);
      return endpoint;
    }

    void MasterClient::registerService(
        const std::string& methodSignature, const qi::detail::EndpointContext& e)
    {
      if (!_isInitialized) {
        return;
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;

      msg << (methodRegisterService);
      msg << (methodSignature);
      msg << (e.endpointID);
      //CTAF:todo
      //_transportClient.send(msg.str(), ret);
    }

    void MasterClient::unregisterService(const std::string& methodSignature)
    {
      if (!_isInitialized) {
        return;
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;

      msg << (methodUnregisterService);
      msg << (methodSignature);
      //CTAF:todo
      //_transportClient.send(msg.str(), ret);
    }

    std::string MasterClient::locateTopic(const std::string& methodSignature, const qi::detail::EndpointContext& e) {
      if (!_isInitialized) {
        return "";
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;

      msg << (methodLocateTopic);
      msg << (methodSignature);
      msg << (e.endpointID);
      //CTAF:todo
      //_transportClient.send(msg.str(), ret);
      qi::DataStream retSer(ret);
      std::string endpoint;
      retSer >> (endpoint);
      return endpoint;
    }

    bool MasterClient::topicExists(const std::string& signature)
    {
      if (!_isInitialized) {
        return false;
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;

      msg << (methodTopicExists);
      msg << (signature);
      //CTAF:todo
      //_transportClient.send(msg.str(), ret);
      qi::DataStream retSer(ret);
      bool exists;
      retSer >> (exists);
      return exists;
    }

    void MasterClient::registerTopic(
        const std::string& signature, const bool& isManyToMany,
        const qi::detail::EndpointContext& e)
    {
      if (!_isInitialized) {
        return;
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;

      msg << (methodRegisterTopic);
      msg << (signature);
      msg << (isManyToMany);
      msg << (e.endpointID);
      //CTAF:todo
      //_transportClient.send(msg.str(), ret);
    }

    void MasterClient::registerTopicParticipant(
      const std::string& signature,
      const std::string& endpointID)
    {
      if (!_isInitialized) {
        return;
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;

      msg << (methodRegisterTopicParticipant);
      msg << (signature);
      msg << (endpointID);
      //CTAF:todo
      //_transportClient.send(msg.str(), ret);
    }

    void MasterClient::unregisterTopic(
      const std::string& signature,
      const qi::detail::EndpointContext& e)
    {
      if (!_isInitialized) {
        return;
      }
      qi::transport::Buffer ret;
      qi::DataStream msg;

      msg << (methodUnregisterTopic);
      msg << (signature);
      msg << (e.endpointID);
      //CTAF:todo
//      _transportClient.send(msg.str(), ret);
    }
  }
}


