/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	AL_MESSAGING_CLIENT_HPP_
# define   	AL_MESSAGING_CLIENT_HPP_

# include <string>
# include <alcore/alptr.h>
# include <alcommon-ng/transport/client.hpp>

namespace AL {
  namespace Messaging {

    class ResultDefinition;
    class CallDefinition;

    class Client {
    public:
      Client(const std::string &address);

    public:
      template<typename R, typename T1>
      R call(const T1 &p1);

      template<typename R, typename T1, typename T2>
      R call(const T1 &p1, const T2 &p2);

      //TODO: ...

    //protected:
    public:
      boost::shared_ptr<ResultDefinition> send(CallDefinition &def);

    protected:
      AL::Transport::Client *_client;
    };
  }
}



#endif	    /* !AL_MESSAGING_CLIENT_HPP_ */
