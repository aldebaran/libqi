/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qimessaging/c/server_c.h>
#include <qimessaging/c/qi_c.h>
#include <qimessaging/functor.hpp>

#include <cstring>
#include <cstdlib>

qi_server_t *qi_server_create(const char *QI_UNUSED(name)) {
//  qi::Server *pserver = new qi::ServerImpl(name);
//  return reinterpret_cast<qi_server_t *>(pserver);
  return NULL;
}

void         qi_server_destroy(qi_server_t *QI_UNUSED(server)) {
//  qi::detail::ServerImpl  *pserver  = reinterpret_cast<qi::detail::ServerImpl *>(server);
//  delete pserver;
}

void         qi_server_connect(qi_server_t *QI_UNUSED(server), const char *QI_UNUSED(address)) {
//  qi::detail::ServerImpl  *pserver  = reinterpret_cast<qi::detail::ServerImpl *>(server);
//  pserver->connect(address);
}


void         qi_server_advertise_service(qi_server_t *QI_UNUSED(server), const char *QI_UNUSED(methodSignature), BoundMethod QI_UNUSED(func), void *QI_UNUSED(data)) {
//  qi::detail::ServerImpl  *pserver  = reinterpret_cast<qi::detail::ServerImpl *>(server);
//  CFunctor *fun = new CFunctor(methodSignature, func, data);
//  pserver->advertiseService(methodSignature, fun);
}

void         qi_server_unadvertise_service(qi_server_t *QI_UNUSED(server), const char *QI_UNUSED(methodSignature)) {
//  qi::detail::ServerImpl  *pserver  = reinterpret_cast<qi::detail::ServerImpl *>(server);
//  pserver->unadvertiseService(methodSignature);
}
