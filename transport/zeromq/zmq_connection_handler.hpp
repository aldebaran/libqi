/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef AL_MESSAGING_CONNECTION_HANDLER_HPP_
#define AL_MESSAGING_CONNECTION_HANDLER_HPP_

#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/transport/common/runnable.hpp>
#include <alcommon-ng/transport/common/server_response_delegate.hpp>
#include <alcommon-ng/transport/common/ondatadelegate.hpp>
#include <string>

namespace AL {
  namespace Transport {

/**
 * @brief A connection handler created for each new incoming connection and pushed to
 * the thread pool.
 */
    class ZMQConnectionHandler : public Runnable {
    public:
      ZMQConnectionHandler(const std::string &msg, OnDataDelegate *sdelegate, internal::ServerResponseDelegate* rdelegate, void *data);
      virtual ~ZMQConnectionHandler ();
      virtual void run ();

    private:
      void                             *_data;
      std::string                       _msg;
      OnDataDelegate                   *_callbackdelegate;
      internal::ServerResponseDelegate *_responsedelegate;
    };

  }
}

#endif /* !AL_MESSAGING_CONNECTION_HANDLER_HPP_ */
