/*
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#ifndef _QIMESSAGING_SIGNAL_H_
#define _QIMESSAGING_SIGNAL_H_

#include <qic/api.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void (*qi_signal_callback_t)(qi_value_t* params, void *data);


  QIC_API qi_signal_t*      qi_signal_create();
  QIC_API void              qi_signal_destroy(qi_signal_t* sig);
  QIC_API unsigned int      qi_signal_connect(qi_signal_t* sig, qi_signal_callback_t f, void *user_data);
  QIC_API bool              qi_signal_disconnect(qi_signal_t* sig, unsigned int l);
  QIC_API bool              qi_signal_disconnect_all(qi_signal_t* sig);
  QIC_API void              qi_signal_trigger(qi_signal_t* sig, qi_value_t* param);

#ifdef __cplusplus
}
#endif

#endif // _QIMESSAGING_SIGNAL_H_
