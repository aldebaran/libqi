/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	AL_MESSAGING_CLIENT_HPP_
# define   	AL_MESSAGING_CLIENT_HPP_

# include <string>
# include <alcommon-ng/transport/client.hpp>
# include <alcommon-ng/transport/zeromq/zmqclient.hpp>
# include <alcommon-ng/serialization/serializer.hpp>

namespace AL {
  namespace Messaging {

    template<typename T, typename R>
    class Client {
    public:
      Client(const std::string &address)
      {
        _client = new AL::Transport::ZMQClient(address);
      }

      R send(const T &def)
      {
        std::string tosend = AL::Serialization::Serializer::serialize(def);
        std::string torecv;

        _client->send(tosend, torecv);
        return AL::Serialization::Serializer::deserialize<R>(torecv);
      }

    protected:
      AL::Transport::Client *_client;
    };

  }
}



#endif	    /* !AL_MESSAGING_CLIENT_HPP_ */
