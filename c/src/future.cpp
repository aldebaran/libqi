/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#include <list>
#include <vector>

#include <qic/future.h>
#include <qic/object.h>
#include <qic/value.h>
#include <qi/future.hpp>
#include "future_p.h"
#include "value_p.h"


qiLogCategory("qipy.future");

qi_promise_t* qi_promise_create()
{
  qi::Promise<qi::GenericValue> *pr = new qi::Promise<qi::GenericValue>();
  return (qi_promise_t *) pr;
}

void  qi_promise_destroy(qi_promise_t *pr)
{
  qi::Promise<qi::GenericValue>  *promise = qi_promise_cpp(pr);
  delete promise;
  promise = 0;
}

void qi_promise_set_value(qi_promise_t *pr, qi_value_t *value)
{
  qi::Promise<qi::GenericValue>  *promise = qi_promise_cpp(pr);
  //TODO: Take?
  promise->setValue(qi_value_cpp(value));
}

void qi_promise_set_error(qi_promise_t *pr, const char *error)
{
  qi::Promise<qi::GenericValue>* promise = qi_promise_cpp(pr);
  promise->setError(error);
}

qi_future_t* qi_promise_get_future(qi_promise_t *pr)
{
  qi::Promise<qi::GenericValue>* promise = qi_promise_cpp(pr);
  return qi_cpp_promise_get_future(*promise);
}

void    qi_future_destroy(qi_future_t *fu)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  qiLogInfo() << "qi_future_destroy(" << fu << ")";
  //TODO: the GenericValue should be somehow destroyed
  delete fut;
  fut = 0;
}

QIC_API qi_future_t*  qi_future_clone(qi_future_t* fu) {
  qi::Future<qi::GenericValue>* fut = qi_future_cpp(fu);
  qi::Future<qi::GenericValue>* clo = new qi::Future<qi::GenericValue>();
  *clo = *fut;
  return (qi_future_t *)clo;
}

void    qi_future_add_callback(qi_future_t *fu, qi_future_callback_t cb, void *miscdata)
{
  qiLogInfo() << "qi_future_add_cb(" << fu << ")";
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  fut->connect(boost::bind<void>(cb, fu, miscdata));
}

void    qi_future_wait(qi_future_t *fu)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  fut->wait();
}

int     qi_future_has_error(qi_future_t *fu)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  return fut->hasError();
}

int     qi_future_is_ready(qi_future_t *fu)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  return fut->isReady();
}

qi_value_t *qi_future_get_value(qi_future_t *fu)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);

  if (fut->hasError())
    return 0;
  qi_value_t *val = qi_value_create("");
  qi::GenericValue &gv = qi_value_cpp(val);
  gv = fut->value();
  return val;
}

const char*         qi_future_get_error(qi_future_t *fu)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  return strdup(fut->error().c_str());
}

//Syntaxic Sugar

long long qi_future_get_int64(qi_future_t* fut, int *err) {
  if (qi_future_has_error(fut)) {
    if (err)
      *err = 1;
    return 0;
  }
  qi_value_t* val = qi_future_get_value(fut);
  long long ret;
  ret = qi_value_get_int64(val, err);
  qi_value_destroy(val);
  qi_future_destroy(fut);
  return ret;
}

unsigned long long qi_future_get_uint64(qi_future_t* fut, int *err) {
  if (qi_future_has_error(fut)) {
    if (err)
      *err = 1;
    return 0;
  }
  qi_value_t* val = qi_future_get_value(fut);
  unsigned long long ret;
  ret = qi_value_get_uint64(val, err);
  qi_value_destroy(val);
  qi_future_destroy(fut);
  return ret;
}


const char* qi_future_get_string(qi_future_t* fut) {
  if (qi_future_has_error(fut)) {
    return 0;
  }
  qi_value_t* val = qi_future_get_value(fut);
  const char *str;
  str = qi_value_get_string(val);
  qi_value_destroy(val);
  qi_future_destroy(fut);
  return str;
}

qi_object_t* qi_future_get_object(qi_future_t* fut) {
  if (qi_future_has_error(fut)) {
    return 0;
  }
  qi_value_t* val = qi_future_get_value(fut);
  qi_object_t* obj;
  obj = qi_value_get_object(val);
  qi_value_destroy(val);
  qi_future_destroy(fut);
  return obj;
}
