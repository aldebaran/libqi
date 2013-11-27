/*
**
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qic/session.h>
#include <qic/object.h>
#include <qic/future.h>
#include "future_p.h"

#include <string.h>
#include <qimessaging/session.hpp>
#include <qimessaging/serviceinfo.hpp>

#ifdef __cplusplus
extern "C"
{
#endif

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

char * qi_session_url(qi_session_t* session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  return qi::os::strdup(s->url().str().c_str());

}

int qi_session_set_identity(qi_session_t *session, char *key, char *crt)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  return (int)s->setIdentity(key, crt);
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

qi_future_t* qi_session_listen_standalone(qi_session_t *session, const char *address)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  return qi_future_wrap(s->listenStandalone(address));
}

qi_value_t*  qi_session_endpoints(qi_session_t *session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  std::vector<qi::Url> eps = s->endpoints();
  std::vector<std::string> epss;

  std::vector<qi::Url>::iterator it;
  for (it = eps.begin(); it != eps.end(); ++it) {
    epss.push_back(it->str());
  }
  qi_value_t *val = qi_value_create("");
  qi::AnyValue &gvp = qi_value_cpp(val);
  gvp = qi::AnyValue::from(epss);
  return val;
}

qi_future_t* qi_session_register_service(qi_session_t *session, const char *name, qi_object_t *object)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  qi::AnyObject  *obj = reinterpret_cast<qi::AnyObject *>(object);
  return qi_future_wrap(s->registerService(name, *obj));
}

qi_future_t* qi_session_unregister_service(qi_session_t *session, unsigned int idx)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  return qi_future_wrap(s->unregisterService(idx));
}

#ifdef __cplusplus
}
#endif
