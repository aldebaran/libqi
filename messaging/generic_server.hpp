/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	AL_MESSAGING_SERVER_HPP_
# define   	AL_MESSAGING_SERVER_HPP_

#include <string>
#include <alcommon-ng/messaging/generic_messagehandler.hpp>
#include <alcommon-ng/transport/common/datahandler.hpp>
#include <alcommon-ng/transport/common/threadable.hpp>
#include <alcommon-ng/transport/zeromq/zmqsimpleserver.hpp>
#include <alcommon-ng/transport/transport.hpp>
#include <alcommon-ng/serialization/serializer.hpp>
#include <allog/allog.h>

namespace AL {

  namespace Messaging {

    template<typename T, typename R>
    class GenericServer : public AL::Transport::Threadable,
      public AL::Transport::DataHandler
    {
    public:
      bool initOK;

      GenericServer(): initOK(false) {} 

      void serve(const std::string &address)
      {
        try {
          _server = new AL::Transport::ZMQSimpleServer(address);
          _server->setDataHandler(this);
          initOK = true;
        } catch(const std::exception& e) {
          alserror << "Failed to create transport server for address: " << address << " Reason:" << e.what();
          throw(e);
        }
      }

      virtual void run()
      {
        if (initOK) {
          _server->run();
        }
      }

      virtual void setMessageHandler(GenericMessageHandler<T, R>* callback) {
        _onMessageDelegate = callback;
      }

      virtual GenericMessageHandler<T, R>* getMessageHandler() {
        return _onMessageDelegate;
      }


    protected:
      virtual void onData(const std::string &dataIn, std::string &dataOut)
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
        _onMessageDelegate->onMessage(in, out);

        assert(out);
        dataOut = AL::Serialization::Serializer::serialize(out);
      }

    protected:
      std::string           _serverAddress;
      GenericMessageHandler<T, R>  *_onMessageDelegate;
      AL::Transport::Server *_server;
    };

  }
}



#endif	    /* !AL_MESSAGING_SERVER_HPP_ */
