/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   QI_MESSAGING_SERVER_HPP_
# define  QI_MESSAGING_SERVER_HPP_

#include <string>
#include <qi/messaging/i_generic_message_handler.hpp>
#include <qi/transport/common/i_data_handler.hpp>
#include <qi/transport/common/i_threadable.hpp>
#include <qi/transport/zeromq/zmqsimpleserver.hpp>
#include <qi/transport/transport.hpp>
#include <qi/serialization/serializer.hpp>
#include <allog/allog.h>

namespace qi {

  namespace messaging {

    template<typename T, typename R>
    class GenericServer :
      public qi::transport::IThreadable,
      public qi::transport::IDataHandler
    {
    public:
      bool initOK;

      GenericServer(): initOK(false) {}

      void serve(const std::string &address)
      {
        fServerAddress = address;
        try {
          fTransportServer = new qi::transport::ZMQSimpleServer(address);
          fTransportServer->setDataHandler(this);
          initOK = true;
        } catch(const std::exception& e) {
          alserror << "Failed to create transport server for address: " << address << " Reason:" << e.what();
          throw(e);
        }
      }

      virtual void run()
      {
        if (initOK) {
          fTransportServer->run();
        }
      }

      virtual void setMessageHandler(IGenericMessageHandler<T, R>* dataHandler) {
        fMessageHandler = dataHandler;
      }

      virtual IGenericMessageHandler<T, R>* getMessageHandler() {
        return fMessageHandler;
      }


    protected:
      virtual void dataHandler(const std::string &dataIn, std::string &dataOut)
      {
        if (! initOK ) {
          alserror << "Attempt to use an uninitialized server";
        }
        // "dataIn" contains a serialized version of "in",
        // we will de-serialize this and pass it to the method
        // then serialize the "out" result to "dataOut"
        T in;
        qi::serialization::Serializer::deserialize(dataIn, in);

        R out;
        fMessageHandler->messageHandler(in, out);

        dataOut = qi::serialization::Serializer::serialize(out);
      }

    protected:
      std::string                   fServerAddress;
      IGenericMessageHandler<T, R>* fMessageHandler;
      qi::transport::Server       * fTransportServer;
    };

  }
}



#endif  // QI_MESSAGING_SERVER_HPP_
