/*
** Author(s):
**  - Jean-Charles DELAY <jdelay@aldebaran-robotics.com>
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_TRANSPORT_SERVER_CALLBACK_DELEGATE_HPP_
#define AL_MESSAGING_TRANSPORT_SERVER_CALLBACK_DELEGATE_HPP_

#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/serialization/result_definition.hpp>

namespace AL {
  namespace Transport {

    /** Interface used by transport's server to delegate the data handling
      */
    class DataHandler {
    public:
      //return 0 if no result is expected
      virtual void onData(const std::string &data, std::string &result) = 0;
    };

  }
}

#endif /* !AL_MESSAGING_TRANSPORT_SERVER_CALLBACK_DELEGATE_HPP_ */
