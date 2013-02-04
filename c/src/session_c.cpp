/*
**
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <string.h>
#include <qimessaging/c/qi_c.h>
#include <qimessaging/c/error_c.h>
#include <qimessaging/c/session_c.h>
#include <qimessaging/session.hpp>
#include <qimessaging/serviceinfo.hpp>
#include "error_p.h"

qi_session_t *qi_session_create()
{
  qi::Session *session = new qi::Session();

  return reinterpret_cast<qi_session_t*>(session);
}

bool qi_session_is_connected(qi_session_t *session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  return s->isConnected();
}

bool qi_session_connect(qi_session_t *session, const char *address)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  try
  {
    qi::Future<bool> fut = s->connect(address);
    fut.wait();
    if (fut.hasError() || fut.value() == false)
    {
      qi_c_set_error(fut.error().c_str());
      return false;
    }
    return true;
  }
  catch (std::runtime_error &e)
  {
    qi_c_set_error(e.what());
    return false;
  }
  catch (std::bad_alloc &e)
  {
    qi_c_set_error(e.what());
    return false;
  }
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
    qi_c_set_error("Session is not valid.");
    return 0;
  }

  qi_object_t *obj = qi_object_create();
  qi::ObjectPtr &o = *(reinterpret_cast<qi::ObjectPtr *>(obj));

  try
  {
    o = s->service(name);
    if (!o) {
      qi_object_destroy(obj);
      return 0;
    }
    return obj;
  }
  catch (std::runtime_error &e)
  {
    qi_c_set_error(e.what());
    return 0;
  }
}

const char** qi_session_get_services(qi_session_t *session)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);
  std::vector<qi::ServiceInfo> services;
  size_t length = 0;

  try {
    services = s->services();
    length = services.size();
  } catch (std::runtime_error& e) {
    return 0;
  }

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
  qi::ObjectPtr  *obj = reinterpret_cast<qi::ObjectPtr *>(object);

  try
  {
    return s->registerService(name, *obj);
  }
  catch (std::runtime_error &e)
  {
    qi_c_set_error(e.what());
  }

  return 0;
}

void qi_session_unregister_service(qi_session_t *session, unsigned int idx)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(session);

  s->unregisterService(idx);
}
