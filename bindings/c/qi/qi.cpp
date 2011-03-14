/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qi/qi.h>
#include <qi/messaging.hpp>
#include <qi/messaging/src/client_impl.hpp>

qi_context_t *qi_context_create() {
  qi::Context *pctx = new qi::Context();
  return (qi_context_t *)pctx;
}

void qi_context_destroy(qi_context_t *ctx) {
  if (!ctx)
    return;
  qi::Context *pctx = (qi::Context *)ctx;
  delete pctx;
}


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




