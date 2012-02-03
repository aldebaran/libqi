/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_MESSAGE_H_
#define _QIMESSAGING_MESSAGE_H_

#include <qimessaging/api.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void qi_message_t;


  QIMESSAGING_API qi_message_t *qi_message_create();
  QIMESSAGING_API void          qi_message_destroy(qi_message_t *msg);

  QIMESSAGING_API void          qi_message_write_bool(qi_message_t   *msg, const char b);
  QIMESSAGING_API void          qi_message_write_char(qi_message_t   *msg, const char b);
  QIMESSAGING_API void          qi_message_write_int(qi_message_t    *msg, const int i);
  QIMESSAGING_API void          qi_message_write_float(qi_message_t  *msg, const float f);
  QIMESSAGING_API void          qi_message_write_double(qi_message_t *msg, const double d);
  QIMESSAGING_API void          qi_message_write_string(qi_message_t *msg, const char *);
  QIMESSAGING_API void          qi_message_write_raw(qi_message_t    *msg, const char *, unsigned int size);

  QIMESSAGING_API char   qi_message_read_bool(qi_message_t   *msg);
  QIMESSAGING_API char   qi_message_read_char(qi_message_t   *msg);
  QIMESSAGING_API int    qi_message_read_int(qi_message_t    *msg);
  QIMESSAGING_API float  qi_message_read_float(qi_message_t  *msg);
  QIMESSAGING_API double qi_message_read_double(qi_message_t *msg);
  QIMESSAGING_API char  *qi_message_read_string(qi_message_t *msg);
  QIMESSAGING_API char  *qi_message_read_raw(qi_message_t    *msg, unsigned int *size);


#ifdef __cplusplus
}
#endif

#endif  // _QIMESSAGING_MESSAGE_H_
