/*
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#include <qic/value.h>
#include "value_p.h"
#include "object_p.h"

qi_value_t* qi_value_create(const char *signature)
{
  qi::GenericValue* v;

  if (!strcmp(signature, ""))
    v = new qi::GenericValue();
  else {
    //TODO: check signature correctness
    qi::Type *type = qi::Type::fromSignature(signature);
    v = new qi::GenericValue(type);
  }
  return (qi_value_t*)v;
}

void qi_value_destroy(qi_value_t* val)
{
  qi::GenericValue *v = &qi_value_cpp(val);
  delete v;
}

void        qi_value_swap(qi_value_t* dest, qi_value_t* src)
{
  qi::GenericValue *s = &qi_value_cpp(src);
  qi::GenericValue *d = &qi_value_cpp(dest);
  s->swap(*d);
}

qi_value_t* qi_value_copy(qi_value_t* src)
{
  qi::GenericValue *s = &qi_value_cpp(src);
  qi::GenericValue *d = new qi::GenericValue;
  *d = s;
  return (qi_value_t*)d;
}

int        qi_value_set_string(qi_value_t *msg, const char *s)
{
  qi::GenericValue &gv = qi_value_cpp(msg);
  try {
    gv.setString(s);
  } catch (std::runtime_error &) {
    return 0;
  }
  return 1;
}

const char* qi_value_get_string(qi_value_t *msg)
{
  qi::GenericValue &gv = qi_value_cpp(msg);
  try {
    return qi::os::strdup(gv.toString().c_str());
  } catch (std::runtime_error &) {
  }
  return 0;
}

int       qi_value_set_uint64(qi_value_t *msg, unsigned long long ul) {
  qi::GenericValue &gv = qi_value_cpp(msg);
  try {
    //TODO: setUInt
    gv.setInt(ul);
  } catch (std::runtime_error&) {
    return 0;
  }
  return 1;
}

unsigned long long qi_value_get_uint64(qi_value_t *msg, int *err) {
  qi::GenericValue &gv = qi_value_cpp(msg);
  //todo: check for signatures matches
  if (err)
    *err = 0;
  try {
    return gv.toInt();
  } catch (std::runtime_error &) {
    if (err)
      *err = 1;
  }
  return 0;
}

int       qi_value_set_int64(qi_value_t  *msg, long long l) {
  qi::GenericValue &gv = qi_value_cpp(msg);
  try {
    gv.setInt(l);
  } catch (std::runtime_error &) {
    return 0;
  }
  return 1;
}

long long  qi_value_get_int64(qi_value_t  *msg, int *err) {
  qi::GenericValue &gv = qi_value_cpp(msg);
  //todo: check for signatures matches
  if (err)
    *err = 0;
  try {
    return gv.toInt();
  } catch (std::runtime_error &) {
    if (err)
      *err = 1;
  }
  return 0;
}

int        qi_value_tuple_set(qi_value_t *msg, unsigned int idx, qi_value_t *value)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  qi::GenericValue &val = qi_value_cpp(value);

  if (idx >= container.size()) {
    return 0;
  }
  container[idx].set(val);
  return 1;
}

qi_value_t*  qi_value_tuple_get(qi_value_t *msg, unsigned int idx, int *err) {
  qi::GenericValue &container = qi_value_cpp(msg);
  if (idx >= container.size()) {
    *err = 1;
    return 0;
  }
  qi_value_t* ret = qi_value_create("");
  qi::GenericValue &gv = qi_value_cpp(ret);
  gv = container[idx];
  if (err)
    *err = 0;
  return ret;
}

qi_value_kind_t qi_value_get_kind(qi_value_t* value) {
  if (!value)
    return QI_VALUE_KIND_UNKNOWN;
  qi::GenericValue &val = qi_value_cpp(value);
  return (qi_value_kind_t)(val.kind());
}

int          qi_value_tuple_size(qi_value_t *msg)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  return container.size();
}

int          qi_value_list_set(qi_value_t *msg, unsigned int idx, qi_value_t *value)
{
  return qi_value_tuple_set(msg, idx, value);
}

qi_value_t*  qi_value_list_get(qi_value_t *msg, unsigned int idx, int *err)
{
  return qi_value_tuple_get(msg, idx, err);
}

int          qi_value_list_size(qi_value_t *msg)
{
  return qi_value_tuple_size(msg);
}

qi_object_t* qi_value_get_object(qi_value_t* val)
{
  qi::GenericValuePtr &gv = qi_value_cpp(val);
  qi::ObjectPtr obj = gv.as<qi::ObjectPtr>();
  return qi_object_create_from(obj);
}
