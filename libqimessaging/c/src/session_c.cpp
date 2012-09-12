/*
**
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <stdlib.h>
#include <string.h>
#include <qimessaging/c/qi_c.h>
#include <qimessaging/c/session_c.h>
#include <qimessaging/session.hpp>
#include <qimessaging/service_info.hpp>

qi_session_t *qi_session_create()
{
  qi::Session *session = new qi::Session();

  return reinterpret_cast<qi_session_t*>(session);
}

bool qi_session_connect(qi_session_t *session, const char *address)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  return s->connect(address);
}

void qi_session_destroy(qi_session_t *session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  delete s;
}

void qi_session_close(qi_session_t *session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  s->close();
}

int qi_session_get_service_id(qi_session_t *session, const char *service_name)
{
  qi::Session *s = reinterpret_cast<qi::Session *>(session);
  std::vector<qi::ServiceInfo>      services;
  std::vector<qi::ServiceInfo>::iterator it;

  services = s->services();
  for (it = services.begin(); it != services.end(); ++it)
    if ((*it).name().compare(service_name) == 0)
      return (*it).serviceId();

  return 0;
}

qi_object_t *qi_session_get_service(qi_session_t *session, const char *name)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  if (!s)
  {
    printf("session not valid.\n");
    return 0;
  }

  qi_object_t *obj = qi_object_create();
  qi::Object *o = reinterpret_cast<qi::Object *>(obj);
  *o = s->service(name);
  if (!o->isValid()) {
    qi_object_destroy(obj);
    return 0;
  }
  return obj;
}

const char** qi_session_get_services(qi_session_t *session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  std::vector<qi::ServiceInfo> services = s->services();
  size_t length = services.size();
  const char **result = static_cast<const char**>(malloc((length + 1) * sizeof(char *)));
  unsigned int i = 0;

  for (i = 0; i < length; i++)
  {
    result[i] = strdup(services[i].name().c_str());
  }

  result[i] = NULL;

  return result;
}

void qi_session_free_services_list(const char **list)
{
  while (*list != 0)
  {
    free((void*) (*list));
  }

  free((void*) (*list));
}

bool qi_session_listen(qi_session_t *session, const char *address)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  return s->listen(address);
}

int qi_session_register_service(qi_session_t *session, const char *name, qi_object_t *object)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  qi::Object  *obj = reinterpret_cast<qi::Object *>(object);

  return s->registerService(name, *obj);
}

void qi_session_unregister_service(qi_session_t *session, unsigned int idx)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  s->unregisterService(idx);
}
