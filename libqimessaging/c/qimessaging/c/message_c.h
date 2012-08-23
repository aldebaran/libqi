/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_MESSAGE_H_
#define _QIMESSAGING_MESSAGE_H_

#include <qimessaging/c/api_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct qi_message_t_s {} qi_message_t;

  QIMESSAGING_API qi_message_t *qi_message_create();
  QIMESSAGING_API void          qi_message_destroy(qi_message_t *msg);

  QIMESSAGING_API void          qi_message_write_bool(qi_message_t *msg, char b);
  QIMESSAGING_API void          qi_message_write_int8(qi_message_t   *msg, char b);
  QIMESSAGING_API void          qi_message_write_int16(qi_message_t   *msg, short b);
  QIMESSAGING_API void          qi_message_write_int32(qi_message_t   *msg, int b);
  QIMESSAGING_API void          qi_message_write_int64(qi_message_t   *msg, long long b);
  QIMESSAGING_API void          qi_message_write_uint8(qi_message_t   *msg, unsigned char b);
  QIMESSAGING_API void          qi_message_write_uint16(qi_message_t   *msg, unsigned short b);
  QIMESSAGING_API void          qi_message_write_uint32(qi_message_t   *msg, unsigned int b);
  QIMESSAGING_API void          qi_message_write_uint64(qi_message_t   *msg, unsigned long long b);
  QIMESSAGING_API void          qi_message_write_float(qi_message_t  *msg, float f);
  QIMESSAGING_API void          qi_message_write_double(qi_message_t *msg, double d);
  QIMESSAGING_API void          qi_message_write_string(qi_message_t *msg, const char *s);
  QIMESSAGING_API void          qi_message_write_raw(qi_message_t    *msg, const char *s, unsigned int size);

  QIMESSAGING_API char               qi_message_read_bool(qi_message_t   *msg);
  QIMESSAGING_API char               qi_message_read_int8(qi_message_t   *msg);
  QIMESSAGING_API short              qi_message_read_int16(qi_message_t   *msg);
  QIMESSAGING_API int                qi_message_read_int32(qi_message_t   *msg);
  QIMESSAGING_API long long          qi_message_read_int64(qi_message_t   *msg);
  QIMESSAGING_API unsigned char      qi_message_read_uint8(qi_message_t   *msg);
  QIMESSAGING_API unsigned short     qi_message_read_uint16(qi_message_t   *msg);
  QIMESSAGING_API unsigned int       qi_message_read_uint32(qi_message_t   *msg);
  QIMESSAGING_API unsigned long long qi_message_read_uint64(qi_message_t   *msg);
  QIMESSAGING_API float              qi_message_read_float(qi_message_t  *msg);
  QIMESSAGING_API double             qi_message_read_double(qi_message_t *msg);
  QIMESSAGING_API char              *qi_message_read_string(qi_message_t *msg);
  QIMESSAGING_API char              *qi_message_read_raw(qi_message_t    *msg, unsigned int *size);

#ifdef __cplusplus
}
#endif

#endif  // _QIMESSAGING_MESSAGE_H_
