/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_SESSION_H_
#define _QIMESSAGING_SESSION_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct qi_session_t_s {} qi_session_t;

  //forward declaration
  typedef struct qi_object_t_s     qi_object_t;

  QIMESSAGING_API qi_session_t *qi_session_create();
  QIMESSAGING_API void          qi_session_destroy(qi_session_t *session);

  // P.R. FIXME: make asynchronous, return a future, make a _sync version
  QIMESSAGING_API bool          qi_session_connect(qi_session_t *session, const char   *address);
  QIMESSAGING_API bool          qi_session_listen(qi_session_t *session, const char *address);
  QIMESSAGING_API int           qi_session_register_service(qi_session_t *session, const char *name, qi_object_t *object);
  QIMESSAGING_API void          qi_session_unregister_service(qi_session_t *session, unsigned int idx);

  // P.R. FIXME: make asynchronous, return a future, make a _sync version
  QIMESSAGING_API qi_object_t  *qi_session_get_service(qi_session_t *session, const char *name);
  QIMESSAGING_API void          qi_session_close(qi_session_t *session);
  QIMESSAGING_API const char**  qi_session_get_services(qi_session_t *session);
  QIMESSAGING_API void          qi_session_free_services_list(const char **list);


#ifdef __cplusplus
}
#endif

#endif  // _QIMESSAGING_CLIENT_H_
