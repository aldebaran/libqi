/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	AL_MESSAGING_SERVER_HPP_
# define   	AL_MESSAGING_SERVER_HPP_

# include <string>
# include <alcommon-ng/messaging/messagehandler.hpp>
# include <alcommon-ng/transport/common/datahandler.hpp>
# include <alcommon-ng/transport/common/threadable.hpp>

namespace AL {
  namespace Transport {
    class Server;
  }

  namespace Messaging {

    //class ResultHandler;

    class Server : public AL::Transport::Threadable,
      public AL::Transport::DataHandler
    {
    public:
      Server(const std::string &address);
      virtual void run();

    public:
      virtual void setMessageHandler(MessageHandler* callback) {
        _onMessageDelegate = callback;
      }
      
      virtual MessageHandler* getMessageHandler() {
        return _onMessageDelegate;
      }

    protected:
      virtual void onData(const std::string &data, std::string &result);

    protected:
      std::string            _serverAddress;
      MessageHandler        *_onMessageDelegate;
      AL::Transport::Server *_server;
    };

  }
}



#endif	    /* !AL_MESSAGING_SERVER_HPP_ */
