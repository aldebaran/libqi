/*
** Author(s):
**  - Cedric GESTES      <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef LIBIPPC_CONNECTIONHANDLER_HPP_
#define LIBIPPC_CONNECTIONHANDLER_HPP_

#include <alippc/serialization/call_definition.hpp>
#include <alippc/transport/common/runnable.hpp>
#include <alippc/transport/common/server_response_delegate.hpp>
#include <alippc/transport/common/server_command_delegate.hpp>
#include <string>

namespace AL {
  namespace Messaging {

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
