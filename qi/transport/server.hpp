/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner  <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef QI_TRANSPORT_SERVER_HPP_
# define QI_TRANSPORT_SERVER_HPP_

#include <string>
#include <qi/log.hpp>
#include <qi/transport/message_handler.hpp>
#include <qi/transport/detail/serverimpl.hpp>
#include <qi/transport/zeromq/zmqsimpleserver.hpp>


namespace qi {
  namespace transport {

    template<typename TRANSPORT>
    class GenericTransportServer
    {
    public:
      GenericTransportServer(): isInitialized(false) {}

      void serve(const std::string &address)
      {
        fServerAddress = address;
        try {
          fTransportServer = new TRANSPORT(address);
          isInitialized = true;
        } catch(const std::exception& e) {
          qisError << "Failed to create transport server for address: " << address << " Reason:" << e.what() << std::endl;
          throw(e);
        }
      }

      virtual void run()
      {
        if (isInitialized) {
          fTransportServer->run();
        }
      }

      virtual void setMessageHandler(MessageHandler* dataHandler) {
        fTransportServer->setDataHandler(dataHandler);
      }

      virtual MessageHandler* getMessageHandler() {
        return fTransportServer->getDataHandler();
      }


    protected:
      bool isInitialized;

    protected:
      //TODO: do we need that?
      std::string                        fServerAddress;
      qi::transport::detail::ServerImpl *fTransportServer;
    };

    typedef GenericTransportServer<qi::transport::detail::ZMQSimpleServerImpl> ZMQServer;
    typedef ZMQServer Server;
  }

}

#endif  // QI_TRANSPORT_SERVER_HPP_
