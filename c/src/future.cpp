/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/
#include <qic/future.h>
#include <qic/object.h>
#include <qic/value.h>
#include "future_p.h"
#include "value_p.h"

#include <list>
#include <vector>

#include <qi/future.hpp>



qiLogCategory("qi.future");

#ifdef __cplusplus
extern "C"
{
#endif

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

qi_future_t*  qi_future_clone(qi_future_t* fu)
{
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

void    qi_future_wait(qi_future_t *fu, int timeout)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  fut->wait(timeout);
}

int qi_future_has_error(qi_future_t *fu, int timeout)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  try {
    return fut->hasError(timeout);
  } catch (qi::FutureException &fe) {
    return -fe.state();
  }
}

int qi_future_has_value(qi_future_t *fu, int timeout)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  try {
    return fut->hasValue(timeout);
  } catch (qi::FutureException &fe) {
    return -fe.state();
  }
}

int qi_future_is_finished(qi_future_t *fu)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  return fut->isFinished();
}

int qi_future_is_running(qi_future_t *fu)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  return fut->isRunning();
}

int qi_future_is_canceled(qi_future_t *fu)
{
  qi::Future<qi::GenericValue> *fut = qi_future_cpp(fu);
  return fut->isCanceled();
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

long long qi_future_get_int64_default(qi_future_t* fut, long long def) {
  long long res;
  qi_value_t* val = 0;
  if (!qi_future_has_value(fut, QI_FUTURETIMEOUT_INFINITE)) {
    res = def;
    goto clean;
  }
  val = qi_future_get_value(fut);
  if (!qi_value_get_int64(val, &res)) {
    res = def;
    goto clean;
  }

clean:
  qi_value_destroy(val);
  qi_future_destroy(fut);
  return res;
}

unsigned long long qi_future_get_uint64_default(qi_future_t* fut, unsigned long long def) {
  unsigned long long res;
  qi_value_t* val = 0;
  if (!qi_future_has_value(fut, QI_FUTURETIMEOUT_INFINITE)) {
    res = def;
    goto cleanup;
  }
  val = qi_future_get_value(fut);
  if (!qi_value_get_uint64(val, &res)) {
    res = def;
    goto cleanup;
  }

cleanup:
  qi_value_destroy(val);
  qi_future_destroy(fut);
  return res;
}


const char* qi_future_get_string(qi_future_t* fut) {
  const char *str = 0;
  qi_value_t* val = 0;
  if (!qi_future_has_value(fut, QI_FUTURETIMEOUT_INFINITE)) {
    str = 0;
    goto clean;
  }
  val = qi_future_get_value(fut);
  str = qi_value_get_string(val);
clean:
  qi_value_destroy(val);
  qi_future_destroy(fut);
  return str;
}

qi_object_t* qi_future_get_object(qi_future_t* fut) {
  qi_object_t *str = 0;
  qi_value_t* val = 0;
  if (!qi_future_has_value(fut, QI_FUTURETIMEOUT_INFINITE)) {
    str = 0;
    goto clean;
  }
  val = qi_future_get_value(fut);
  str = qi_value_get_object(val);
clean:
  qi_value_destroy(val);
  qi_future_destroy(fut);
  return str;
}


#ifdef __cplusplus
}
#endif
