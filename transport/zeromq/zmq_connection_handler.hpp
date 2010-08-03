/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef LIBIPPC_CONNECTIONHANDLER_HPP_
#define LIBIPPC_CONNECTIONHANDLER_HPP_

#include <alcommon-ng/serialization/call_definition.hpp>
#include <alcommon-ng/transport/common/runnable.hpp>
#include <alcommon-ng/transport/common/server_response_delegate.hpp>
#include <alcommon-ng/transport/common/server_command_delegate.hpp>
#include <string>

namespace AL {
  namespace Transport {

/**
 * @brief A connection handler created for each new incoming connection and pushed to
 * the thread pool.
 */
    class ZMQConnectionHandler : public Runnable {
    public:
      ZMQConnectionHandler(AL::ALPtr<CallDefinition> def, ServerCommandDelegate *sdelegate, internal::ServerResponseDelegate* rdelegate, void *data);
      virtual ~ZMQConnectionHandler ();
      virtual void run ();

    private:
      void                              *data;
      AL::ALPtr<CallDefinition>         def;
      ServerCommandDelegate             *callbackdelegate;
      internal::ServerResponseDelegate  *responsedelegate;
    };

  }
}

#endif /* !LIBIPPC_CONNECTIONHANDLER_HPP_ */
