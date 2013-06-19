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

  //this follow qi::TypeKind
  typedef enum {
    QI_VALUE_KIND_VOID     = 0,
    QI_VALUE_KIND_INT      = 1,
    QI_VALUE_KIND_FLOAT    = 2,
    QI_VALUE_KIND_STRING   = 3,
    QI_VALUE_KIND_LIST     = 4,
    QI_VALUE_KIND_MAP      = 5,
    QI_VALUE_KIND_OBJECT   = 6,
    QI_VALUE_KIND_POINTER  = 7,
    QI_VALUE_KIND_TUPLE    = 8,
    QI_VALUE_KIND_DYNAMIC  = 9,
    QI_VALUE_KIND_RAW      = 10,
    QI_VALUE_KIND_UNKNOWN  = 11,
    QI_VALUE_KIND_ITERATOR = 12,
    QI_VALUE_KIND_INVALID  = 13,
  } qi_value_kind_t;


  //# CTOR/DTOR/COPY
  QIC_API qi_value_t* qi_value_create(const char *sig);
  QIC_API void        qi_value_destroy(qi_value_t *v);
  QIC_API int         qi_value_reset(qi_value_t* value, const char *sig);
  QIC_API void        qi_value_swap(qi_value_t* dest, qi_value_t* src);
  QIC_API qi_value_t* qi_value_copy(qi_value_t* src);

  //# TYPE
  QIC_API qi_value_kind_t qi_value_get_kind(qi_value_t* value);
  QIC_API const char*     qi_value_get_signature(qi_value_t* value, int resolveDynamics);

  //# INT/UINT
  QIC_API int                qi_value_set_uint64(qi_value_t *value, unsigned long long ul);
  QIC_API int                qi_value_get_uint64(qi_value_t *value, unsigned long long *defvalue);
  QIC_API unsigned long long qi_value_get_uint64_default(qi_value_t *value, unsigned long long defvalue);
  QIC_API int                qi_value_set_int64(qi_value_t  *value, long long l);
  QIC_API int                qi_value_get_int64(qi_value_t  *value, long long *l);
  QIC_API long long          qi_value_get_int64_default(qi_value_t  *value, long long defvalue);

  //# FLOAT/DOUBLE
  QIC_API int qi_value_set_float(qi_value_t  *msg, float f);
  QIC_API int qi_value_get_float(qi_value_t  *msg, float *f);
  QIC_API float qi_value_get_float_default(qi_value_t  *msg, float defvalue);

  QIC_API int qi_value_set_double(qi_value_t *msg, double d);
  QIC_API int qi_value_get_double(qi_value_t *msg, double *d);
  QIC_API double qi_value_get_double_default(qi_value_t *msg, double defvalue);

  //# STRING
  QIC_API int         qi_value_set_string(qi_value_t *value, const char *s);
  //return a copy that should be freed
  QIC_API const char* qi_value_get_string(qi_value_t *value);


  //# LIST
  QIC_API int          qi_value_list_set(qi_value_t *container, unsigned int idx, qi_value_t *element);
  QIC_API qi_value_t*  qi_value_list_get(qi_value_t *container, unsigned int idx);
  QIC_API int          qi_value_list_push_back(qi_value_t* msg, qi_value_t* val);
  QIC_API int          qi_value_list_size(qi_value_t *container);

  //# MAP
  QIC_API unsigned int qi_value_map_size(qi_value_t *msg);
  QIC_API int          qi_value_map_set(qi_value_t *msg, qi_value_t *key, qi_value_t *value);
  QIC_API qi_value_t*  qi_value_map_get(qi_value_t *msg, qi_value_t *key);

  //# OBJECT
  QIC_API qi_object_t* qi_value_get_object(qi_value_t* value);
  QIC_API int          qi_value_set_object(qi_value_t* value, qi_object_t *obj);

  //# TUPLE
  QIC_API int          qi_value_tuple_set(qi_value_t *container, unsigned int idx, qi_value_t *element);
  QIC_API qi_value_t*  qi_value_tuple_get(qi_value_t *container, unsigned int idx);
  QIC_API int          qi_value_tuple_size(qi_value_t *container);

  //# DYNAMIC
  QIC_API int          qi_value_dynamic_set(qi_value_t *container, qi_value_t* value);
  QIC_API qi_value_t*  qi_value_dynamic_get(qi_value_t *container);

  //# RAW
  QIC_API int          qi_value_raw_set(qi_value_t* value, const char* data, int size);
  QIC_API int          qi_value_raw_get(qi_value_t* value, const char**data, int *size);


#ifdef __cplusplus
}
#endif

#endif  // _QIMESSAGING_MESSAGE_H_
