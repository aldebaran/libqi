/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	AL_MESSAGING_SERVER_HPP_
# define   	AL_MESSAGING_SERVER_HPP_

#include <string>
#include <alcommon-ng/messaging/messagehandler.hpp>
#include <alcommon-ng/transport/common/datahandler.hpp>
#include <alcommon-ng/transport/common/threadable.hpp>
#include <alcommon-ng/transport/zeromq/zmqsimpleserver.hpp>
#include <alcommon-ng/transport/transport.hpp>
#include <alcommon-ng/serialization/serializer.hpp>

// temporary
#include <alcommon-ng/messaging/call_definition_serialization.hxx>
#include <alcommon-ng/messaging/result_definition_serialization.hxx>

namespace AL {

  namespace Messaging {

    template<typename T, typename R>
    class Server : public AL::Transport::Threadable,
      public AL::Transport::DataHandler
    {
    public:
      Server(const std::string &address)
      {
        _server = new AL::Transport::ZMQSimpleServer(address);
        _server->setDataHandler(this);
      }

      virtual void run()
      {
        _server->run();
      }

      virtual void setMessageHandler(MessageHandler<T, R>* callback) {
        _onMessageDelegate = callback;
      }

      virtual MessageHandler<T, R>* getMessageHandler() {
        return _onMessageDelegate;
      }

    protected:
      virtual void onData(const std::string &data, std::string &result)
      {
        T def = AL::Serialization::Serializer::deserialize<T>(data);
        boost::shared_ptr<R> res;

        res = _onMessageDelegate->onMessage(def);
        assert(res);
        result = AL::Serialization::Serializer::serialize(*res);
      }

    protected:
      std::string           _serverAddress;
      MessageHandler<T, R>  *_onMessageDelegate;
      AL::Transport::Server *_server;
    };

  }
}



#endif	    /* !AL_MESSAGING_SERVER_HPP_ */
