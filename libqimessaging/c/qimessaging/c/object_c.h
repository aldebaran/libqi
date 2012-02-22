/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_OBJECT_H_
#define _QIMESSAGING_OBJECT_H_

#include <qimessaging/c/message_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct {} qi_object_t;

  typedef void (*BoundMethod)(const char *complete_signature, qi_message_t *msg, qi_message_t *ret, void *data);

  qi_object_t *qi_object_create(const char *name);
  void         qi_object_destroy(qi_object_t *object);
  void         qi_object_connect(qi_object_t *object, const char *address);

  int          qi_object_register_method(qi_object_t *object, const char *name, BoundMethod func, void *data);
  int          qi_object_register_signal(qi_object_t *object, const char *name);
  int          qi_object_register_slot(qi_object_t *object, const char *name);
  int          qi_object_register_property(qi_object_t *object, const char *name);

  int          qi_object_set_property(qi_object_t *object, const char *name);
  int          qi_object_get_property(qi_object_t *object, const char *name);

#ifdef __cplusplus
}
#endif

#endif
