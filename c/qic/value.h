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

#include <qic/api.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct qi_value_t_s qi_value_t;

  //forward declaration
  typedef struct qi_object_t_s     qi_object_t;

  QIC_API qi_value_t* qi_value_create(const char *sig);
  QIC_API void        qi_value_destroy(qi_value_t *v);

  QIC_API void        qi_value_swap(qi_value_t* dest, qi_value_t* src);
  QIC_API qi_value_t* qi_value_copy(qi_value_t* src);

//  QIMESSAGING_API void        qi_value_set_signature(const char *sig);
//  QIMESSAGING_API char*       qi_value_get_signature();

  QIC_API int                qi_value_set_uint64(qi_value_t *value, unsigned long long ul);
  QIC_API unsigned long long qi_value_get_uint64(qi_value_t *value, int *err);

  QIC_API int        qi_value_set_int64(qi_value_t  *value, long long l);
  QIC_API long long  qi_value_get_int64(qi_value_t  *value, int *err);

//  QIMESSAGING_API void       qi_value_set_float(qi_value_t  *msg, float f);
//  QIMESSAGING_API void       qi_value_set_double(qi_value_t *msg, double d);

  QIC_API int         qi_value_set_string(qi_value_t *value, const char *s);
  //return a copy that should be freed
  QIC_API const char* qi_value_get_string(qi_value_t *value);

  QIC_API qi_object_t* qi_value_get_object(qi_value_t* value);

  //  QIMESSAGING_API void       qi_value_set_raw(qi_value_t    *msg, const char *s, unsigned int size);

//  QIMESSAGING_API unsigned int qi_value_list_size(qi_value_t *msg);
//  QIMESSAGING_API void         qi_value_list_set(qi_value_t *msg, unsigned int idx, qi_value_t *value);
//  QIMESSAGING_API void         qi_value_list_get(qi_value_t *msg, unsigned int idx, qi_value_t *result);
//  //missing iterator
//  //missing insert

//  QIMESSAGING_API unsigned int qi_value_map_size(qi_value_t *msg);
//  QIMESSAGING_API void         qi_value_map_set(qi_value_t *msg, qi_value_t *key, qi_value_t *value);
//  QIMESSAGING_API void         qi_value_map_get(qi_value_t *msg, unsigned int idx, qi_value_t *result);
//  //missing iterator
//  //missing insert?

//  QIMESSAGING_API unsigned int qi_value_tuple_size(qi_value_t *msg);
  QIC_API int          qi_value_tuple_set(qi_value_t *container, unsigned int idx, qi_value_t *element);
  QIC_API qi_value_t*  qi_value_tuple_get(qi_value_t *container, unsigned int idx, int *err);
  QIC_API int          qi_value_tuple_size(qi_value_t *container);

  QIC_API int          qi_value_list_set(qi_value_t *container, unsigned int idx, qi_value_t *element);
  QIC_API qi_value_t*  qi_value_list_get(qi_value_t *container, unsigned int idx);
  QIC_API int          qi_value_list_size(qi_value_t *container);

//  QIMESSAGING_API void         qi_value_tuple_get(qi_value_t *msg, unsigned int idx, qi_value_t *result);

//  QIMESSAGING_API bool         qi_value_value_get(qi_value_t *msg, qi_value_t *result);
//  QIMESSAGING_API bool         qi_value_value_set(qi_value_t *msg, qi_value_t *result);

////object




//  QIMESSAGING_API char               qi_message_read_uint64(qi_value_t   *msg);
//  QIMESSAGING_API char               qi_message_read_int64(qi_value_t   *msg);
//  QIMESSAGING_API float              qi_message_read_float(qi_value_t  *msg);
//  QIMESSAGING_API double             qi_message_read_double(qi_value_t *msg);
//  /*! \warning Must be free using qi_message_free_string */
//  QIMESSAGING_API char              *qi_message_read_string(qi_value_t *msg);
//  /*! \warning Must be free using qi_message_free_string */
//  QIMESSAGING_API char              *qi_message_read_raw(qi_value_t    *msg, unsigned int *size);

//  QIMESSAGING_API unsigned int       qi_message_read_list_size(qi_value_t *msg);
//  QIMESSAGING_API unsigned int       qi_message_read_map_size(qi_value_t *msg);
//  QIMESSAGING_API unsigned int       qi_message_read_tuple_size(qi_value_t *msg);

//  QIMESSAGING_API void               qi_message_free_raw(char *raw);
//  QIMESSAGING_API void               qi_message_free_string(char *str);
#ifdef __cplusplus
}
#endif

#endif  // _QIMESSAGING_MESSAGE_H_
