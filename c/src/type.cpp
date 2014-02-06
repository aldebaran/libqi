/*
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2014 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qic/type.h>
#include <qitype/details/type.hpp>
#include "value_p.h"
#include "object_p.h"

qiLogCategory("qi.c");

#ifdef __cplusplus
extern "C"
{
#endif

qi_type_t*   qi_type_of_kind(qi_type_kind_t kind) {
  return (qi_type_t*) qi::makeTypeOfKind(qi::TypeKind(kind));
}

qi_type_t* qi_type_int(int issigned, int bytelen) {
  return (qi_type_t*) qi::makeIntType(issigned, bytelen);
}

qi_type_t* qi_type_float(int bytelen) {
  return (qi_type_t*) qi::makeFloatType(bytelen);
}


qi_type_t*   qi_type_list_of(qi_type_t *element) {
  return (qi_type_t*)qi::makeListType((qi::TypeInterface*)element);
}

qi_type_t*   qi_type_map_of(qi_type_t *key, qi_type_t *value) {
  return (qi_type_t*)qi::makeMapType((qi::TypeInterface*)key, (qi::TypeInterface*)value);
}

qi_type_t*   qi_type_tuple_of(int elementc, qi_type_t *elementv[]) {
  std::vector<qi::TypeInterface*> tv(elementc);

  for (int i = 0; i < elementc; ++i)
    tv[i] = (qi::TypeInterface*)(elementv[i]);
  return (qi_type_t*)qi::makeTupleType(tv);
}

void         qi_type_destroy(qi_type_t *) {
  //nothing to do. TypeInterface* should not be destructed.
  //should we tell that to the user ?
}

qi_type_kind_t qi_type_get_kind(qi_type_t* type) {
  return (qi_type_kind_t)((qi::TypeInterface*)(type))->kind();
}

qi_type_t*   qi_type_get_key(qi_type_t* type) {
  qi::detail::AnyType at((qi::TypeInterface*)type);
  return (qi_type_t*)at.key().type();
}

qi_type_t*   qi_type_get_value(qi_type_t* type) {
  qi::detail::AnyType at((qi::TypeInterface*)type);
  return (qi_type_t*)at.element().type();
}

int   qi_type_get_element_count(qi_type_t* type) {
  return 0;
}

qi_type_t*   qi_type_get_element(qi_type_t* type, int index) {
  return 0;
}



#ifdef __cplusplus
}
#endif
