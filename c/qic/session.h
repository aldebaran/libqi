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

#include <qic/api.h>

#ifdef __cplusplus
extern "C"
{
#endif



  QIC_API qi_session_t *qi_session_create();
  QIC_API void          qi_session_destroy(qi_session_t *session);

  QIC_API qi_future_t*  qi_session_connect(qi_session_t *session, const char   *address);
  QIC_API int           qi_session_is_connected(qi_session_t *session);
  QIC_API qi_future_t*  qi_session_listen(qi_session_t *session, const char *address);
  QIC_API qi_future_t*  qi_session_register_service(qi_session_t *session, const char *name, qi_object_t *object);
  QIC_API qi_future_t*  qi_session_unregister_service(qi_session_t *session, unsigned int idx);

  QIC_API qi_future_t*  qi_session_get_service(qi_session_t *session, const char *name);
  QIC_API qi_future_t*  qi_session_close(qi_session_t *session);
  QIC_API qi_future_t*  qi_session_get_services(qi_session_t *session);


#ifdef __cplusplus
}
#endif

#endif  // _QIMESSAGING_CLIENT_H_
