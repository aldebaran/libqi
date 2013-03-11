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
  qi::GenericValuePtr* v;

  if (!strcmp(signature, ""))
    v = new qi::GenericValuePtr();
  else {
    //TODO: check signature correctness
    qi::Type *type = qi::Type::fromSignature(signature);
    v = new qi::GenericValuePtr(type);
  }
  return (qi_value_t*)v;
}

void qi_value_destroy(qi_value_t* val)
{
  qi::GenericValuePtr *v = &qi_value_cpp(val);
  if (v)
    v->destroy();
  delete v;
}

void        qi_value_swap(qi_value_t* dest, qi_value_t* src)
{
  qi::GenericValuePtr *s = &qi_value_cpp(src);
  qi::GenericValuePtr *d = &qi_value_cpp(dest);

  qi::GenericValuePtr t = *s;
  *s = *d;
  *d = t;
}

qi_value_t* qi_value_copy(qi_value_t* src)
{
  qi::GenericValuePtr *s = &qi_value_cpp(src);
  qi_value_t *dest = qi_value_create("");
  qi::GenericValuePtr *d = &qi_value_cpp(dest);
  *d = s->clone();
  return dest;
}

int        qi_value_set_string(qi_value_t *msg, const char *s)
{
  qi::GenericValuePtr &gv = qi_value_cpp(msg);
  gv.destroy();
  //todo: check that types matches
  gv = qi::GenericValuePtr::from(std::string(s)).clone();
  return true;
}

const char* qi_value_get_string(qi_value_t *msg)
{
  qi::GenericValuePtr &gv = qi_value_cpp(msg);
  //todo: check for types
  return qi::os::strdup(gv.asString().c_str());
}

int       qi_value_set_uint64(qi_value_t *msg, unsigned long long ul) {
  qi::GenericValuePtr &gv = qi_value_cpp(msg);
  //todo: check for signature matches...
  gv = qi::GenericValuePtr::from(ul).clone();
  return true;
}

unsigned long long qi_value_get_uint64(qi_value_t *msg, int *err) {
  qi::GenericValuePtr &gv = qi_value_cpp(msg);
  //todo: check for signatures matches
  if (err)
    *err = 0;
  return gv.asInt();
}

int       qi_value_set_int64(qi_value_t  *msg, long long l) {
  qi::GenericValuePtr &gv = qi_value_cpp(msg);
  //todo: check for signatures matches
  gv = qi::GenericValuePtr::from(l).clone();
  return true;
}

long long  qi_value_get_int64(qi_value_t  *msg, int *err) {
  qi::GenericValuePtr &gv = qi_value_cpp(msg);
  //todo: check for signatures matches
  if (err)
    *err = 0;
  return gv.asInt();
}

int        qi_value_tuple_set(qi_value_t *msg, unsigned int idx, qi_value_t *value)
{
  qi::GenericValuePtr &container = qi_value_cpp(msg);
  qi::GenericValuePtr &val = qi_value_cpp(value);
  std::vector<qi::GenericValuePtr> vt = container.asTuple().get();
  if (idx >= vt.size())
    return false;
  qi::GenericValuePtr &gvp = vt.at(idx);
  //gvp.destroy();
  gvp = val.clone();
  //TODO: check for type?
  container.asTuple().set(vt);
  return true;
}

qi_value_t*  qi_value_tuple_get(qi_value_t *msg, unsigned int idx, int *err) {
  qi::GenericValuePtr &container = qi_value_cpp(msg);
  std::vector<qi::GenericValuePtr> vt = container.asTuple().get();
  if (idx >= vt.size()) {
    *err = 1;
    return 0;
  }
  qi_value_t* ret = qi_value_create("");
  qi::GenericValuePtr &gv = qi_value_cpp(ret);
  gv.destroy();
  gv = vt[idx].clone();
  if (err)
    *err = 0;
  return ret;
}

qi_value_kind_t qi_value_get_kind(qi_value_t* value) {
  if (!value)
    return QI_VALUE_KIND_UNKNOWN;
  qi::GenericValuePtr &val = qi_value_cpp(value);
  return (qi_value_kind_t)(val.kind());
}

int          qi_value_tuple_size(qi_value_t *msg)
{
  qi::GenericValuePtr &container = qi_value_cpp(msg);
  std::vector<qi::GenericValuePtr> vt = container.asTuple().get();
  return vt.size();
}

int          qi_value_list_set(qi_value_t *msg, unsigned int idx, qi_value_t *value)
{
  qi::GenericValuePtr &container = qi_value_cpp(msg);
  //TODO
  return -1;
}

qi_value_t*  qi_value_list_get(qi_value_t *msg, unsigned int idx)
{
  qi::GenericValuePtr &container = qi_value_cpp(msg);
  qi::GenericListIteratorPtr it;
  it = container.asList().begin();
  for (unsigned int i = 0; i < idx; ++i) {
    ++it;
  }
  qi_value_t* ret = qi_value_create("");
  qi::GenericValuePtr &gv = qi_value_cpp(ret);
  gv = (*it).clone();
  it.destroy();
  //TODO
  return ret;
}

int          qi_value_list_size(qi_value_t *msg)
{
  qi::GenericValuePtr &container = qi_value_cpp(msg);
  return container.asList().size();
}

qi_object_t* qi_value_get_object(qi_value_t* val)
{
  qi::GenericValuePtr &gv = qi_value_cpp(val);
  qi::ObjectPtr obj = gv.as<qi::ObjectPtr>();
  return qi_object_create_from(obj);
}
