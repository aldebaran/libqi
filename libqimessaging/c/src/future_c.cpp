/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#include <list>
#include <vector>

#include <qimessaging/c/future_c.h>
#include <qi/future.hpp>
#include <qimessaging/c/object_c.h>

#include "future_c_p.h"
#include "object_c_p.h"

qi_promise_t* qi_promise_create()
{
  qi::Promise<void *> *pr = new qi::Promise<void *>();

  return (qi_promise_t *) pr;
}

void  qi_promise_destroy(qi_promise_t *pr)
{
  qi::Promise<void *>  *promise = reinterpret_cast<qi::Promise<void *> *>(pr);

  delete promise;
}

void qi_promise_set_value(qi_promise_t *pr, void *value)
{
  qi::Promise<void *>  *promise = reinterpret_cast<qi::Promise<void *> *>(pr);

  promise->setValue(value);
}

void qi_promise_set_error(qi_promise_t *pr, const char *error)
{
  qi::Promise<void *>  *promise = reinterpret_cast<qi::Promise<void *> *>(pr);

  promise->setError(error);
}

qi_future_t* qi_promise_get_future(qi_promise_t *pr)
{
  qi::Promise<void *> *promise = reinterpret_cast<qi::Promise<void *> *>(pr);
  qi_future_data_t*     data = new qi_future_data_t;

  data->future = new qi::Future<void *>();
  *data->future = promise->future();

  return (qi_future_t *) data;
}

void    qi_future_destroy(qi_future_t *fut)
{
  qi_future_data_t      *data = reinterpret_cast<qi_future_data_t*>(fut);
  std::list<FutureCallbackForwarder *>::iterator it;

  for (it = data->callbacks.begin(); it != data->callbacks.end(); ++it)
    delete (*it);

}

void    qi_future_set_callback(qi_future_t *fut, qi_future_callback_t cb, void *miscdata)
{
  qi_future_data_t      *data = reinterpret_cast<qi_future_data_t*>(fut);
  FutureCallbackForwarder *forwarder = new FutureCallbackForwarder();

  data->callbacks.push_back(forwarder);
  forwarder->addCallback(cb);
  data->future->connect(boost::bind<void>(&FutureCallbackForwarder::onResult, forwarder, _1, miscdata));
}

void    qi_future_wait(qi_future_t *fut)
{
  qi_future_data_t      *data = reinterpret_cast<qi_future_data_t*>(fut);

  data->future->wait();
}

int     qi_future_is_error(qi_future_t *fut)
{
  qi_future_data_t      *data = reinterpret_cast<qi_future_data_t*>(fut);

  return data->future->hasError();
}

int     qi_future_is_ready(qi_future_t *fut)
{
  qi_future_data_t      *data = reinterpret_cast<qi_future_data_t*>(fut);

  return data->future->isReady();
}

void *qi_future_get_value(qi_future_t *fut)
{
  qi_future_data_t      *data = reinterpret_cast<qi_future_data_t*>(fut);

  return (qi_message_t *) data->future->value();
}

const char*         qi_future_get_error(qi_future_t *fut)
{
  qi_future_data_t      *data = reinterpret_cast<qi_future_data_t*>(fut);

  return data->future->error().c_str();
}
