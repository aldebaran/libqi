/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   __QI_TRANSPORT_SERVER_HPP__
#define   __QI_TRANSPORT_SERVER_HPP__

#include <string>
#include <qi/log.hpp>
#include <qi/transport/message_handler.hpp>
#include <qi/transport/detail/server_impl.hpp>
#include <qi/transport/detail/zmq/zmq_simple_server.hpp>

namespace qi {
  namespace transport {

    template<typename TRANSPORT>
    class GenericTransportServer
    {
    public:
      GenericTransportServer(): _isInitialized(false) {}

      void serve(const std::string &address) {
        std::vector<std::string> v;
        v.push_back(address);
        serve(v);
      }

      void serve(const std::vector<std::string> &addresses)
      {
        //for(unsigned int i = 0 ; i< addresses.size(); ++i) {
        //  qisInfo << "* GenericTransportServer:serve " << addresses[i] << std::endl;
        //}
        try {
          _transportServer = new TRANSPORT(addresses);
          _isInitialized = true;
        } catch(const std::exception& e) {
          qisError << "Failed to create transport server for addresses:";
          for(unsigned int i = 0 ; i< addresses.size(); ++i) {
            qisError << " " << addresses[i] << std::endl;
          }
          qisError << " Reason:" << e.what() << std::endl;
          throw(e);
        }
      }

      virtual void run()
      {
        if (_isInitialized) {
          _transportServer->run();
        }
      }

      virtual void setMessageHandler(MessageHandler* dataHandler) {
        _transportServer->setDataHandler(dataHandler);
      }

      virtual MessageHandler* getMessageHandler() {
        return _transportServer->getDataHandler();
      }

      bool isInitialized() {
        return _isInitialized;
      }

    protected:
      bool _isInitialized;
      qi::transport::detail::ServerImpl* _transportServer;
    };

    typedef GenericTransportServer<qi::transport::detail::ZMQSimpleServerImpl> ZMQServer;
    typedef ZMQServer Server;
  }

}

#endif // __QI_TRANSPORT_SERVER_HPP__
