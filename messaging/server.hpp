/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	AL_MESSAGING_SERVER_HPP_
# define   	AL_MESSAGING_SERVER_HPP_

# include <string>

namespace AL {
  namespace Messaging {

    class Server {
    public:
      Server(const std::string &address);

    public:
      //void bind(boost::function fct);

    };

  }
}



#endif	    /* !AL_MESSAGING_SERVER_HPP_ */
