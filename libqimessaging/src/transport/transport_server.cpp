/*
*  Author(s):
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#include <string>
#include <qi/log.hpp>
#include <qimessaging/transport/transport_server.hpp>
#include <qimessaging/transport/transport_context.hpp>
#include <qimessaging/transport/transport_message_handler.hpp>
#include "src/transport/server_backend.hpp"
#include "src/transport/zmq/zmq_simple_server_backend.hpp"

namespace qi {
  namespace transport {

    TransportServer::TransportServer(const TransportServer& rhs) :
      _isInitialized(rhs._isInitialized),
      _transportContext(rhs._transportContext),
      _transportServer(rhs._transportServer) {}

    TransportServer::TransportServer(TransportContext &context)
      : _isInitialized(false),
        _transportContext(context),
        _transportServer(NULL)
    {
    }

    TransportServer::~TransportServer() {
      if (_transportServer != NULL) {
        delete(_transportServer);
        _transportServer = NULL;
      }
    }

    void TransportServer::serve(const std::string &endpoint) {
      std::vector<std::string> v;
      v.push_back(endpoint);
      serve(v);
    }

    void TransportServer::serve(const std::vector<std::string> &endpoints)
    {
      //for(unsigned int i = 0 ; i< addresses.size(); ++i) {
      //  qisInfo << "* GenericTransportServer:serve " << addresses[i] << std::endl;
      //}
      try {
        _transportServer = new detail::ZMQSimpleServerBackend(endpoints, _transportContext.getContext<zmq::context_t>(endpoints[0]));
        _isInitialized = true;
      } catch(const std::exception& e) {
        qiLogError("qimessaging") << "Failed to create transport server for endpoints:";
        for(unsigned int i = 0 ; i< endpoints.size(); ++i) {
          qiLogError("qimessaging") << " " << endpoints[i] << std::endl;
        }
        qiLogError("qimessaging") << " Reason:" << e.what() << std::endl;
        throw(e);
      }
    }

    void TransportServer::run()
    {
      if (_isInitialized) {
        try {
          _transportServer->run();
        } catch(const std::exception& e) {
          qiLogError("qimessaging") << "TransportServer::run exception " << e.what() << std::endl;
        }
      }
    }

    void TransportServer::setMessageHandler(TransportMessageHandler* dataHandler) {
      _transportServer->setMessageHandler(dataHandler);
    }

    TransportMessageHandler* TransportServer::getMessageHandler() {
      return _transportServer->getMessageHandler();
    }

    bool TransportServer::isInitialized() {
      return _isInitialized;
    }

  }

}
