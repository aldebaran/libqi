/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_GENERIC_MESSAGEHANDLER_HPP_
#define AL_MESSAGING_GENERIC_MESSAGEHANDLER_HPP_

namespace AL {
  namespace Messaging {
    /** Use this interface to allow your class receiving message
      */
    template<typename T, typename R>
    class GenericMessageHandler {
    public:
      virtual void onMessage(const T &def, R& result) = 0;
    };
  }
}

#endif  // AL_MESSAGING_GENERIC_MESSAGEHANDLER_HPP_
