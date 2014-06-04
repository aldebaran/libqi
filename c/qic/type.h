/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013, 2014 Aldebaran Robotics
*/

#ifndef _QIC_TYPE_H_
#define _QIC_TYPE_H_

#include <qic/api.h>

#ifdef __cplusplus
extern "C"
{
#endif

//this follow qi::TypeKind
typedef enum {
  QI_TYPE_KIND_INVALID  = -1,
  QI_TYPE_KIND_UNKNOWN  = 0,
  QI_TYPE_KIND_VOID     = 1,
  QI_TYPE_KIND_INT      = 2,
  QI_TYPE_KIND_FLOAT    = 3,
  QI_TYPE_KIND_STRING   = 4,
  QI_TYPE_KIND_LIST     = 5,
  QI_TYPE_KIND_MAP      = 6,
  QI_TYPE_KIND_OBJECT   = 7,
  QI_TYPE_KIND_POINTER  = 8,
  QI_TYPE_KIND_TUPLE    = 9,
  QI_TYPE_KIND_DYNAMIC  = 10,
  QI_TYPE_KIND_RAW      = 11,
  QI_TYPE_KIND_ITERATOR = 13,
  QI_TYPE_KIND_FUNCTION = 14,
  QI_TYPE_KIND_SIGNAL   = 15,
  QI_TYPE_KIND_PROPERTY = 16,
  QI_TYPE_KIND_VARARGS  = 17,
} qi_type_kind_t;


//# Type Creation
QIC_API qi_type_t*   qi_type_of_kind(qi_type_kind_t kind);
QIC_API qi_type_t*   qi_type_int(int issigned, int bytelen);
QIC_API qi_type_t*   qi_type_float(int bytelen);
QIC_API qi_type_t*   qi_type_of_kind(qi_type_kind_t kind);
QIC_API qi_type_t*   qi_type_list_of(qi_type_t *element);
QIC_API qi_type_t*   qi_type_map_of(qi_type_t *key, qi_type_t* value);
QIC_API qi_type_t*   qi_type_tuple_of(int elementc, qi_type_t *elementv[]);

QIC_API void         qi_type_destroy(qi_type_t *);

//# Type Introspection
//return the kind of the type
QIC_API qi_type_kind_t qi_type_get_kind(qi_type_t* type);

//
QIC_API qi_type_t*   qi_type_get_key(qi_type_t* type);

//
QIC_API qi_type_t*   qi_type_get_value(qi_type_t* type);

//Get type of tuple elements
QIC_API int          qi_type_get_element_count(qi_type_t* type);
QIC_API qi_type_t*   qi_type_get_element(qi_type_t* type, int index);

#ifdef __cplusplus
}
#endif

#endif
