/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_ON_MESSAGE_DELEGATE_HPP_
#define AL_MESSAGING_ON_MESSAGE_DELEGATE_HPP_

#include <boost/shared_ptr.hpp>

namespace AL {
  namespace Messaging {

    // TODO: should we expose this interface? (maybe for very advanced use).
    // But there may be a simpler way to works.

    /** Use this interface to allow your class receiving message
      */
    template<typename T, typename R>
    class MessageHandler {
    public:
      //return 0 if no result is expected
      virtual boost::shared_ptr<R> onMessage(const T &def) = 0;
    };

  }
}

#endif /* !AL_MESSAGING_ON_MESSAGE_DELEGATE_HPP_ */
