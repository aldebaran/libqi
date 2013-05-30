/*
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qic/value.h>
#include "value_p.h"
#include "object_p.h"

qiLogCategory("qi.c");

//# GENERIC POD IMPL
template<typename T>
int       qi_value_set_pod(qi_value_t  *msg, T val) {
  qi::GenericValue &gv = qi_value_cpp(msg);
  try {
    gv.set<T>(val);
    return 1;
  } catch (std::runtime_error &) {}
  return 0;
}

template<typename T>
int qi_value_get_pod(qi_value_t *msg, T *result) {
  qi::GenericValue &gv = qi_value_cpp(msg);
  try {
    *result = gv.to<T>();
    return 1;
  } catch (std::runtime_error &) {
  }
  return 0;
}

template<typename T>
T qi_value_get_pod_default(qi_value_t *msg, T defvalue) {
  qi::GenericValue &gv = qi_value_cpp(msg);
  try {
    return gv.to<T>();
  } catch (std::runtime_error &) {
  }
  return defvalue;
}

#ifdef __cplusplus
extern "C"
{
#endif

qi_value_t* qi_value_create(const char *signature)
{
  qi::GenericValue* v;

  if (!signature || !strcmp(signature, ""))
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

int         qi_value_reset(qi_value_t* value, const char *signature)
{
  qi::GenericValue *v = &qi_value_cpp(value);
  if (!signature || !strcmp(signature, ""))
    v->reset();
  else {
    //TODO: check signature correctness
    qi::Type *type = qi::Type::fromSignature(signature);
    v->reset(type);
  }
  return 1;
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
  *d = *s;
  return (qi_value_t*)d;
}


qi_value_kind_t qi_value_get_kind(qi_value_t* value) {
  if (!value)
    return QI_VALUE_KIND_UNKNOWN;
  qi::GenericValue &val = qi_value_cpp(value);
  return (qi_value_kind_t)(val.kind());
}

const char*     qi_value_get_signature(qi_value_t* value, int resolveDynamics)
{
  qi::GenericValue &val = qi_value_cpp(value);
  return qi::os::strdup(val.signature(!!resolveDynamics).toString().c_str());
}

//# UINT64
int qi_value_set_uint64(qi_value_t* container, unsigned long long value)
{ return qi_value_set_pod<unsigned long long>(container, value); }

int qi_value_get_uint64(qi_value_t* container, unsigned long long *result)
{ return qi_value_get_pod<unsigned long long>(container, result); }

unsigned long long qi_value_get_uint64_default(qi_value_t *container, unsigned long long defvalue)
{ return qi_value_get_pod_default<unsigned long long>(container, defvalue); }

//# INT64
int qi_value_set_int64(qi_value_t* container, long long value)
{ return qi_value_set_pod<long long>(container, value); }

int qi_value_get_int64(qi_value_t* container, long long *result)
{ return qi_value_get_pod<long long>(container, result); }

long long qi_value_get_int64_default(qi_value_t *container, long long defvalue)
{ return qi_value_get_pod_default<long long>(container, defvalue); }

//# FLOAT
int qi_value_set_float(qi_value_t* container, float value)
{ return qi_value_set_pod<float>(container, value); }

int qi_value_get_float(qi_value_t* container, float *result)
{ return qi_value_get_pod<float>(container, result); }

float qi_value_get_float_default(qi_value_t *container, float defvalue)
{ return qi_value_get_pod_default<float>(container, defvalue); }

//# DOUBLE
int qi_value_set_double(qi_value_t* container, double value)
{ return qi_value_set_pod<double>(container, value); }

int qi_value_get_double(qi_value_t* container, double *result)
{ return qi_value_get_pod<double>(container, result); }

double qi_value_get_double_default(qi_value_t *container, double defvalue)
{ return qi_value_get_pod_default<double>(container, defvalue); }

//# STRING
int        qi_value_set_string(qi_value_t *container, const char *s)
{
  qi::GenericValue &gv = qi_value_cpp(container);
  try {
    gv.setString(s);
    return 1;
  } catch (std::runtime_error &) {
  }
  return 0;
}

const char* qi_value_get_string(qi_value_t *msg)
{
  if (!msg)
    return 0;
  qi::GenericValue &gv = qi_value_cpp(msg);
  try {
    return qi::os::strdup(gv.toString().c_str());
  } catch (std::runtime_error &) {
  }
  return 0;
}

//# TUPLE
int        qi_value_tuple_set(qi_value_t *msg, unsigned int idx, qi_value_t *value)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  qi::GenericValue &val = qi_value_cpp(value);

  try {
    if (container.kind() != qi::Type::Tuple || idx >= container.size()) {
      return 0;
    }
    container[idx].set(val);
    return 1;
  } catch (std::runtime_error &) {}
  return 0;
}

qi_value_t*  qi_value_tuple_get(qi_value_t *msg, unsigned int idx) {
  qi::GenericValue &container = qi_value_cpp(msg);
  if (container.kind() != qi::Type::Tuple || idx >= container.size()) {
    return 0;
  }
  qi_value_t* ret = qi_value_create("");
  qi::GenericValue &gv = qi_value_cpp(ret);
  gv = container[idx];
  return ret;
}

int          qi_value_tuple_size(qi_value_t *msg)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  if (container.kind() != qi::Type::Tuple) {
    return -1;
  }
  return container.size();
}

//# LIST
int          qi_value_list_set(qi_value_t *msg, unsigned int idx, qi_value_t *value)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  qi::GenericValue &val = qi_value_cpp(value);

  if (container.kind() != qi::Type::List || idx >= container.size()) {
    return 0;
  }
  try {
    container[idx].set(val);
    return 1;
  } catch (std::runtime_error &e) {
    qiLogError() << "Cant set list item at index " << idx << " :" << e.what();
  }
  return 0;
}

qi_value_t*  qi_value_list_get(qi_value_t *msg, unsigned int idx)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  if (container.kind() != qi::Type::List || idx >= container.size()) {
    return 0;
  }
  qi_value_t* ret = qi_value_create("");
  qi::GenericValue &gv = qi_value_cpp(ret);
  gv = container[idx];
  return ret;
}

int  qi_value_list_push_back(qi_value_t *msg, qi_value_t*val)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  qi::GenericValue &gval = qi_value_cpp(val);
  if (container.kind() != qi::Type::List) {
    return 0;
  }
  container.append(gval);
  return 1;
}

int          qi_value_list_size(qi_value_t *msg)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  if (container.kind() != qi::Type::List) {
    return -1;
  }
  return container.size();
}

//# OBJECT
qi_object_t* qi_value_get_object(qi_value_t* val)
{
  qi::GenericValue &gv = qi_value_cpp(val);
  qi::ObjectPtr obj = gv.as<qi::ObjectPtr>();
  return qi_object_create_from(obj);
}

int          qi_value_set_object(qi_value_t* value, qi_object_t *o)
{
  qi::GenericValue &gv = qi_value_cpp(value);
  qi::ObjectPtr &obj = qi_object_cpp(o);
  try {
    gv.set<qi::ObjectPtr>(obj);
    return 1;
  } catch (std::runtime_error&) {}
  return 0;
}

//# DYNAMIC
int          qi_value_dynamic_set(qi_value_t *container, qi_value_t* value)
{
  qi::GenericValue &gvcont = qi_value_cpp(container);
  qi::GenericValue &gvval = qi_value_cpp(value);
  try {
    gvcont.setDynamic(gvval);
    return 1;
  } catch (std::runtime_error&) {}
  return 0;
}

qi_value_t*  qi_value_dynamic_get(qi_value_t *container)
{
  qi::GenericValue &gv = qi_value_cpp(container);
  try {
    qi::GenericValuePtr gvp = gv.asDynamic();
    qi_value_t *ret = qi_value_create("");
    qi::GenericValue &val = qi_value_cpp(ret);
    val = gvp.clone();
    return ret;
  } catch (std::runtime_error&) {}
  return 0;
}

//# MAP
unsigned int qi_value_map_size(qi_value_t *msg)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  if (container.kind() != qi::Type::Map) {
    return -1;
  }
  return container.size();
}

int         qi_value_map_set(qi_value_t *msg, qi_value_t *key, qi_value_t *value)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  qi::GenericValue &k = qi_value_cpp(key);
  qi::GenericValue &val = qi_value_cpp(value);

  if (container.kind() != qi::Type::Map) {
    return 0;
  }
  try {
    container[k].set(val);
    return 1;
  } catch (std::runtime_error &) {}
  return 0;
}

qi_value_t*  qi_value_map_get(qi_value_t *msg, qi_value_t *key)
{
  qi::GenericValue &container = qi_value_cpp(msg);
  qi::GenericValue &k = qi_value_cpp(key);

  qi::GenericValuePtr r;
  if (container.kind() != qi::Type::Map) {
    return 0;
  }
  try {
    r = container._element(k, true);
    qi_value_t* ret = qi_value_create("");
    qi::GenericValue &gv = qi_value_cpp(ret);
    gv = r;
    return ret;
  } catch (std::runtime_error &) {}
  return 0;
}

//# RAW
int          qi_value_raw_set(qi_value_t* value, const char* data, int size){
  return 0;
}

extern "C" int          qi_value_raw_get(qi_value_t* value, const char**data, int *size) {
  return 0;
}

#ifdef __cplusplus
}
#endif
