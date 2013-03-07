/*
**
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <string.h>
#include <qic/session.h>
#include <qic/object.h>
#include <qic/future.h>
#include <qimessaging/session.hpp>
#include <qimessaging/serviceinfo.hpp>
#include "future_p.h"

qi_session_t *qi_session_create()
{
  qi::Session *session = new qi::Session();

  return reinterpret_cast<qi_session_t*>(session);
}

void qi_session_destroy(qi_session_t *session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  delete s;
}

int qi_session_is_connected(qi_session_t *session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  return s->isConnected();
}

qi_future_t* qi_session_connect(qi_session_t *session, const char *address)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  return qi_future_wrap(s->connect(address));
}

qi_future_t* qi_session_close(qi_session_t *session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  return qi_future_wrap(s->close());
}

qi_future_t* qi_session_get_service(qi_session_t *session, const char *name)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  if (!s)
    return 0;
  return qi_future_wrap(s->service(name));
}

qi_future_t* qi_session_get_services(qi_session_t *session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  return qi_future_wrap(s->services());
}

qi_future_t* qi_session_listen(qi_session_t *session, const char *address)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  return qi_future_wrap(s->listen(address));
}

qi_future_t* qi_session_register_service(qi_session_t *session, const char *name, qi_object_t *object)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  qi::ObjectPtr  *obj = reinterpret_cast<qi::ObjectPtr *>(object);
  return qi_future_wrap(s->registerService(name, *obj));
}

qi_future_t* qi_session_unregister_service(qi_session_t *session, unsigned int idx)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  return qi_future_wrap(s->unregisterService(idx));
}
