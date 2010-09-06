/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Chris Kilner <ckilner@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   AL_MESSAGING_SERVER_HPP_
# define  AL_MESSAGING_SERVER_HPP_

#include <string>
#include <alcommon-ng/messaging/i_generic_message_handler.hpp>
#include <alcommon-ng/transport/common/i_data_handler.hpp>
#include <alcommon-ng/transport/common/i_threadable.hpp>
#include <alcommon-ng/transport/zeromq/zmqsimpleserver.hpp>
#include <alcommon-ng/transport/transport.hpp>
#include <alcommon-ng/serialization/serializer.hpp>
#include <allog/allog.h>

namespace AL {

  namespace Messaging {

    template<typename T, typename R>
    class GenericServer :
      public AL::Transport::IThreadable,
      public AL::Transport::IDataHandler
    {
    public:
      bool initOK;

      GenericServer(): initOK(false) {} 

      void serve(const std::string &address)
      {
        fServerAddress = address;
        try {
          fTransportServer = new AL::Transport::ZMQSimpleServer(address);
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
        AL::Serialization::Serializer::deserialize(dataIn, in);

        R out;
        fMessageHandler->messageHandler(in, out);

        dataOut = AL::Serialization::Serializer::serialize(out);
      }

    protected:
      std::string                   fServerAddress;
      IGenericMessageHandler<T, R>* fMessageHandler;
      AL::Transport::Server       * fTransportServer;
    };

  }
}



#endif  // AL_MESSAGING_SERVER_HPP_
