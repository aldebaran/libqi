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

  typedef void (*qi_object_method_t)(const char* complete_signature, qi_value_t* msg, qi_value_t* ret, void* user_data);
  typedef void (*qi_object_signal_callback_t)(qi_value_t* params, void *data);

  QIC_API qi_object_t*         qi_object_create();
  QIC_API void                 qi_object_destroy(qi_object_t* object);

  QIC_API qi_value_t*          qi_object_get_metaobject(qi_object_t* object);

  /*! \return A future that contains a qi_message_t */
  QIC_API qi_future_t*         qi_object_call(qi_object_t *object, const char *signature, qi_value_t* params);

  QIC_API int                  qi_object_event_emit(qi_object_t* object, const char *signature, qi_value_t* params);
  QIC_API qi_future_t*         qi_object_event_connect(qi_object_t* object, const char *signature, qi_object_signal_callback_t f, void* user_data);
  QIC_API qi_future_t*         qi_object_event_disconnect(qi_object_t* object, unsigned long long id);

  //ObjectBuilder
  QIC_API qi_object_builder_t* qi_object_builder_create();
  QIC_API void                 qi_object_builder_destroy(qi_object_builder_t *object);

  QIC_API int                  qi_object_builder_register_method(qi_object_builder_t *object, const char *complete_signature, qi_object_method_t func, void *user_data);
  QIC_API int                  qi_object_builder_register_event(qi_object_builder_t *object, const char *complete_signature);
  QIC_API qi_object_t*         qi_object_builder_get_object(qi_object_builder_t *object_builder);

#ifdef __cplusplus
}
#endif

#endif
