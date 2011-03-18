/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qi/qi.h>
#include <qi/messaging.hpp>
#include <qi/messaging/src/client_impl.hpp>

qi_client_t *qi_client_create(const char *name) {
  qi::detail::ClientImpl *pclient = new qi::detail::ClientImpl(name);
  return static_cast<qi_client_t *>(pclient);
}

qi_client_t *qi_client_create_with_context(const char *name, qi_context_t *ctx) {
  qi::Context *pctx = static_cast<qi::Context *>(ctx);
  qi::detail::ClientImpl *pclient = new qi::detail::ClientImpl(name, pctx);
  return static_cast<qi_client_t *>(pclient);
}

void qi_client_connect(qi_client_t *client, const char *address)
{
  qi::detail::ClientImpl  *pclient  = static_cast<qi::detail::ClientImpl *>(client);
  pclient->connect(address);
}

void      qi_client_destroy(qi_client_t *client)
{
  qi::detail::ClientImpl  *pclient  = static_cast<qi::detail::ClientImpl *>(client);
  delete pclient;
}

void      qi_client_call(qi_client_t *client, const char *method, qi_message_t *msg, qi_message_t *ret)
{
  qi::detail::ClientImpl  *pclient  = static_cast<qi::detail::ClientImpl *>(client);
  qi::serialization::Message *preturn  = (qi::serialization::Message *)ret;
  qi::serialization::Message *pmessage = (qi::serialization::Message *)msg;

  pclient->call(std::string(method), *pmessage, *preturn);
}




