/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_GENERIC_MESSAGEHANDLER_HPP_
#define AL_MESSAGING_GENERIC_MESSAGEHANDLER_HPP_

#include <boost/shared_ptr.hpp>

namespace AL {
  namespace Messaging {

    /** Use this interface to allow your class receiving message
      */
    template<typename T, typename R>
    class GenericMessageHandler {
    public:
      //return 0 if no result is expected
      virtual boost::shared_ptr<R> onMessage(const T &def) = 0;
    };

  }
}

#endif  // AL_MESSAGING_GENERIC_MESSAGEHANDLER_HPP_
