/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	_ALIPPC_TRANSPORT_SERVERBASE_HPP_
# define   	_ALIPPC_TRANSPORT_SERVERBASE_HPP_

# include <althread/altask.h>
# include <alcommon-ng/transport/common/threadable.hpp>
# include <alcommon-ng/transport/common/server_response_delegate.hpp>
# include <alcommon-ng/transport/common/server_command_delegate.hpp>

namespace AL {
  namespace Messaging {

      class ServerBase : public Threadable, public AL::ALTask, public internal::ServerResponseDelegate {
  public:
    ServerBase(const std::string &servername)
      : server_name(servername),
        command_delegate(0)
    {};
    virtual ~ServerBase() {}

    virtual void setCommandDelegate(ServerCommandDelegate* callback) { command_delegate = callback; }

    virtual ServerCommandDelegate *getCommandDelegate() { return command_delegate; }

  protected:
    std::string                     server_name;
    ServerCommandDelegate    *command_delegate;
  };

}
}


#endif	    /* !_ALIPPC_TRANSPORT_ROOT_SERVER_HPP_ */
