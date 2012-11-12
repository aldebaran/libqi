/*
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#ifndef _QIMESSAGING_SIGNAL_H_
#define _QIMESSAGING_SIGNAL_H_

#include <qimessaging/c/api_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void (*qi_signal_callback_t)(void* param);

  typedef struct qi_signal_t_s {}   qi_signal_t;


  QIMESSAGING_API qi_signal_t*      qi_signal_create();
  QIMESSAGING_API void              qi_signal_destroy(qi_signal_t* sig);
  QIMESSAGING_API unsigned int      qi_signal_connect(qi_signal_t* sig, qi_signal_callback_t f);
  QIMESSAGING_API bool              qi_signal_disconnect(qi_signal_t* sig, unsigned int l);
  QIMESSAGING_API bool              qi_signal_disconnect_all(qi_signal_t* sig);
  QIMESSAGING_API void              qi_signal_trigger(qi_signal_t* sig, void* param);


#ifdef __cplusplus
}
#endif

#endif // _QIMESSAGING_SIGNAL_H_
