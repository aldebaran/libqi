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
#include <qi/transport/buffer.hpp>
#include <qi/transport/detail/serverimpl.hpp>
#include <qi/transport/common/i_threadable.hpp>
#include <qi/transport/common/i_server_response_handler.hpp>
#include <qi/transport/common/i_data_handler.hpp>
#include <qi/transport/zeromq/zmqsimpleserver.hpp>
#include <qi/transport/i_message_handler.hpp>


namespace qi {
  namespace transport {

    template< typename Transport = qi::transport::ZMQSimpleServerImpl,
              typename Buffer    = qi::transport::Buffer >
    class _Server: public qi::transport::IThreadable, public qi::transport::IDataHandler
    {
    public:

      _Server(): initOK(false) {}

      void serve(const std::string &address)
      {
        fServerAddress = address;
        try {
          fTransportServer = new Transport(address);
          fTransportServer->setDataHandler(this);
          initOK = true;
        } catch(const std::exception& e) {
          qisError << "Failed to create transport server for address: " << address << " Reason:" << e.what() << std::endl;
          throw(e);
        }
      }

      virtual void run()
      {
        if (initOK) {
          fTransportServer->run();
        }
      }

      virtual void setMessageHandler(IMessageHandler* dataHandler) {
        fMessageHandler = dataHandler;
      }

      virtual IMessageHandler* getMessageHandler() {
        return fMessageHandler;
      }


    protected:
      bool initOK;
      virtual void dataHandler(const std::string &dataIn, std::string &dataOut)
      {
        if (! initOK ) {
          qisError << "Attempt to use an uninitialized server" << std::endl;
        }
        // "dataIn" contains a serialized version of "in",
        // we will de-serialize this and pass it to the method
        // then serialize the "out" result to "dataOut"

        fMessageHandler->messageHandler(dataIn, dataOut);

      }

    protected:
      std::string                        fServerAddress;
      qi::transport::IMessageHandler    *fMessageHandler;
      qi::transport::detail::ServerImpl *fTransportServer;
    };

    typedef _Server<qi::transport::ZMQSimpleServerImpl, std::string> ZMQServer;
    typedef ZMQServer Server;
  }


}

#endif  // QI_TRANSPORT_SERVER_HPP_
