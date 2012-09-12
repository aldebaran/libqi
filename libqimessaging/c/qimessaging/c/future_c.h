/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#ifndef _QIMESSAGING_FUTURE_H_
#define _QIMESSAGING_FUTURE_H_

#include <qimessaging/c/api_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void (*qi_future_callback_t)(const void* value, char success, void *data);

  typedef struct qi_future_t_s  {} qi_future_t;
  typedef struct qi_promise_t_s {} qi_promise_t;

  /* Forward declaration of qi_message_t structure */
  typedef struct qi_message_t_s    qi_message_t;

  QIMESSAGING_API qi_promise_t* qi_promise_create();
  QIMESSAGING_API void          qi_promise_destroy(qi_promise_t *pr);

  QIMESSAGING_API void          qi_promise_set_value(qi_promise_t *pr, void *value);
  QIMESSAGING_API void          qi_promise_set_error(qi_promise_t *pr, const char *error);
  QIMESSAGING_API qi_future_t*  qi_promise_get_future(qi_promise_t *pr);

  QIMESSAGING_API void          qi_future_destroy(qi_future_t *fut);

  QIMESSAGING_API void          qi_future_set_callback(qi_future_t *fut, qi_future_callback_t cb, void *miscdata);
  QIMESSAGING_API void          qi_future_wait(qi_future_t *fut);
  QIMESSAGING_API int           qi_future_is_error(qi_future_t *fut);
  QIMESSAGING_API int           qi_future_is_ready(qi_future_t *fut);
  QIMESSAGING_API void*         qi_future_get_value(qi_future_t *fut);

  QIMESSAGING_API const char*   qi_future_get_error(qi_future_t *fut);

#ifdef __cplusplus
}
#endif

#endif // _QIMESSAGING_FUTURE_H_
