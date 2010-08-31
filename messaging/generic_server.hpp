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

namespace AL {

  namespace Messaging {

    template<typename T, typename R>
    class GenericServer : public AL::Transport::Threadable,
      public AL::Transport::DataHandler
    {
    public:
      GenericServer() {}

      GenericServer(const std::string &address)
      {
        _server = new AL::Transport::ZMQSimpleServer(address);
        _server->setDataHandler(this);
      }

      virtual void run()
      {
        _server->run();
      }

      virtual void setMessageHandler(GenericMessageHandler<T, R>* callback) {
        _onMessageDelegate = callback;
      }

      virtual GenericMessageHandler<T, R>* getMessageHandler() {
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
      GenericMessageHandler<T, R>  *_onMessageDelegate;
      AL::Transport::Server *_server;
    };

  }
}



#endif	    /* !AL_MESSAGING_SERVER_HPP_ */
