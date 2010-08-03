/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010 Aldebaran Robotics
*/

#ifndef   	_ALIPPC_TRANSPORT_CLIENTBASE_HPP_
# define   	_ALIPPC_TRANSPORT_CLIENTBASE_HPP_

# include <string>
# include <alcore/alptr.h>
# include <alippc/serialization/result_definition.hpp>


namespace AL {
  namespace Messaging {

  class ClientBase {
  public:
    ClientBase(const std::string &servername)
      : server_name(servername)
    {}

    virtual AL::ALPtr<ResultDefinition> send(CallDefinition &def) = 0;

  protected:
    std::string         server_name;
  };
}
}

#endif	    /* !_ALIPPC_TRANSPORT_CLIENT_HPP_ */
