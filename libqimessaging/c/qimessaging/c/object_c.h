/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_OBJECT_H_
#define _QIMESSAGING_OBJECT_H_

#include <qimessaging/c/api_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct qi_object_t_s  {} qi_object_t;

  //forward declaration
  typedef struct qi_message_t_s qi_message_t;
  typedef struct qi_future_t_s  qi_future_t;


  typedef void (*qi_object_method_t)(const char *complete_signature, qi_message_t *msg, qi_message_t *ret, void *data);
  QIMESSAGING_API qi_object_t *qi_object_create(const char *name);
  QIMESSAGING_API void         qi_object_destroy(qi_object_t *object);

  QIMESSAGING_API int          qi_object_register_method(qi_object_t *object, const char *complete_signature, qi_object_method_t func, void *data);

  QIMESSAGING_API qi_future_t *qi_object_call(qi_object_t *object, const char *signature, qi_message_t *message);

#ifdef __cplusplus
}
#endif

#endif
