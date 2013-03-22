/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#ifndef _QIMESSAGING_FUTURE_H_
#define _QIMESSAGING_FUTURE_H_

#include <qic/api.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void (*qi_future_callback_t)(qi_future_t* value, void *user_data);
  typedef void (*qi_future_cancel_t)(qi_future_t* value, void *user_data);

  #define QI_FUTURETIMEOUT_NONE     (0)
  #define QI_FUTURETIMEOUT_INFINITE ((int)0x7ffffff)

  QIC_API qi_promise_t* qi_promise_create();
  QIC_API qi_promise_t* qi_promise_cancelable_create(qi_future_cancel_t cb, void *user_data);
  QIC_API void          qi_promise_destroy(qi_promise_t *pr);

  QIC_API void          qi_promise_set_value(qi_promise_t *pr, qi_value_t *value);
  QIC_API void          qi_promise_set_error(qi_promise_t *pr, const char *error);
  QIC_API void          qi_promise_set_canceled(qi_promise_t *pr);
  QIC_API qi_future_t*  qi_promise_get_future(qi_promise_t *pr);

  QIC_API void          qi_future_destroy(qi_future_t *fut);
  QIC_API qi_future_t*  qi_future_clone(qi_future_t* fut);

  QIC_API void          qi_future_add_callback(qi_future_t *fut, qi_future_callback_t cb, void *user_data);
  QIC_API void          qi_future_wait(qi_future_t *fut, int timeout);
  QIC_API int           qi_future_has_error(qi_future_t *fut, int timeout);
  QIC_API int           qi_future_has_value(qi_future_t *fut, int timeout);
  QIC_API int           qi_future_is_running(qi_future_t *fut);
  QIC_API int           qi_future_is_finished(qi_future_t *fut);
  QIC_API int           qi_future_is_canceled(qi_future_t *fut);

  /// the value lifetime depends on the future.
  QIC_API qi_value_t*   qi_future_get_value(qi_future_t *fut);
  QIC_API const char*   qi_future_get_error(qi_future_t *fut);

  // Syntaxic Sugar: value content accessor
  QIC_API long long          qi_future_get_int64_default(qi_future_t* fut, long long def);
  QIC_API unsigned long long qi_future_get_uint64_default(qi_future_t* fut, unsigned long long def);
  //QIC_API float              qi_future_get_float_default(qi_future_t* fut, float def);
  //QIC_API double             qi_future_get_double_default(qi_future_t* fut, double def);
  QIC_API const char*        qi_future_get_string(qi_future_t* fut);
  QIC_API qi_object_t*       qi_future_get_object(qi_future_t* fut);

#ifdef __cplusplus
}
#endif

#endif // _QIMESSAGING_FUTURE_H_
