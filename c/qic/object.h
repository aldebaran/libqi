/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_OBJECT_H_
#define _QIMESSAGING_OBJECT_H_

#include <qic/api.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct qi_object_t_s          {} qi_object_t;
  typedef struct qi_object_builder_t_s  {} qi_object_builder_t;

  //forward declaration
  typedef struct qi_future_t_s qi_future_t;
  typedef struct qi_value_t_s  qi_value_t;

  typedef void (*qi_object_method_t)(const char *complete_signature, qi_value_t *msg, qi_value_t *ret, void *data);

  QIC_API qi_object_t*         qi_object_create();
  QIC_API void                 qi_object_destroy(qi_object_t *object);

  QIC_API qi_value_t*          qi_object_get_metaobject(qi_object_t *object);

  /*! \return A future that contains a qi_message_t */
  QIC_API qi_future_t*         qi_object_call(qi_object_t *object, const char *signature, qi_value_t *params);

  //ObjectBuilder
  QIC_API qi_object_builder_t* qi_object_builder_create();
  QIC_API void                 qi_object_builder_destroy(qi_object_builder_t *object);

  QIC_API int                  qi_object_builder_register_method(qi_object_builder_t *object, const char *complete_signature, qi_object_method_t func, void *data);
  QIC_API qi_object_t*         qi_object_builder_get_object(qi_object_builder_t *object_builder);

#ifdef __cplusplus
}
#endif

#endif
