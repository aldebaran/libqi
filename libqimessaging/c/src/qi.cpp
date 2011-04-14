/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qimessaging/qi.h>
//#include <qimessaging/messaging.hpp>
#include "src/messaging/client_impl.hpp"


// MASTER API
char *qi_master_locate_service(qi_client_t *client, const char *signature)
{
  qi::detail::ClientImpl     *pclient = static_cast<qi::detail::ClientImpl *>(client);
  qi::serialization::Message  message;
  qi::serialization::Message  ret;

  message.writeString("master.locateService::s:ss");
  message.writeString(signature);
  message.writeString(pclient->endpointId());

  pclient->call(std::string("master.locateService::s:ss"), message, ret);
  std::string addr;
  ret.readString(addr);
  return strdup(addr.c_str());
}




