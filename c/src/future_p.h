/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	_QIMESSAGING_FUTURE_C_P_H_
# define   	_QIMESSAGING_FUTURE_C_P_H_

#include <qi/future.hpp>
#include <qitype/genericvalue.hpp>
#include <qic/future.h>
#include <qic/value.h>
#include "value_p.h"
#include <boost/bind.hpp>

inline qi::Promise<qi::GenericValue> *qi_promise_cpp(qi_promise_t *value) {
  return reinterpret_cast<qi::Promise<qi::GenericValue> *>(value);
};

inline qi::Future<qi::GenericValue> *qi_future_cpp(qi_future_t *value) {
  return reinterpret_cast<qi::Future<qi::GenericValue> *>(value);
};

inline qi_future_t* qi_cpp_promise_get_future(qi::Promise<qi::GenericValue> &prom) {
  qi::Future<qi::GenericValue>*  fut = new qi::Future<qi::GenericValue>(prom.future());
  return (qi_future_t *) fut;
}

template <typename T>
inline void qi_future_c_adapter(qi::Future<T> fut, qi_promise_t* prom) {
  if (fut.hasError()) {
    qi_promise_set_error(prom, fut.error().c_str());
    qi_promise_destroy(prom);
    return;
  }
  qi_value_t* val = qi_value_create("");
  qi::GenericValue &gvp = qi_value_cpp(val);
  gvp = qi::GenericValue::from(fut.value());
  qi_promise_set_value(prom, val);
  qi_value_destroy(val);
  qi_promise_destroy(prom);
}

template <typename T>
inline qi_future_t* qi_future_wrap(qi::Future<T> fut) {
  qi_promise_t* prom = qi_promise_create();
  fut.connect(boost::bind<void>(&qi_future_c_adapter<T>, _1, prom));
  return qi_promise_get_future(prom);
}

template <typename T>
inline qi_future_t* qi_future_wrap(qi::FutureSync<T> fut) {
  qi_promise_t* prom = qi_promise_create();
  qi_future_t* fufu = qi_promise_get_future(prom);
  //async to avoid exception handling. we really want a future here, not a FutureSync
  fut.async().connect(boost::bind<void>(&qi_future_c_adapter<T>, _1, prom));
  return fufu;
}


#endif
