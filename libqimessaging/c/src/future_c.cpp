/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebararan Robotics
*/

#include <qimessaging/c/future_c.h>
#include <qimessaging/future.hpp>
#include <list>

//Needed to bridge C callback on C++ Future callbacks;
class FutureCallbackForwarder : public qi::FutureInterface<void *>
{
public:
  FutureCallbackForwarder() {}
  virtual ~FutureCallbackForwarder() {}

  virtual void onFutureFailed(const std::string &error, void *data) { /* nothing to do here */}
  virtual void onFutureFinished(void*const &future, void *data)
  {
    std::list<QiFutureCallback>::iterator it;

    for (it = _callbacks.begin(); it != _callbacks.end(); ++it)
    {
      (*(*it))(future, data);
    }
  }
  void         addCallback(QiFutureCallback callback)
  {
    _callbacks.push_back(callback);
  }

private:
  std::list<QiFutureCallback>   _callbacks;
};

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
  qi::Promise<void *>  *promise = reinterpret_cast<qi::Promise<void *> *>(pr);
  qi::Future<void *>   *fut = new qi::Future<void *>();

  *fut = promise->future();
  return (qi_future_t *) fut;
}

void    qi_future_destroy(qi_future_t *fut)
{
  qi::Future<void *> *future = reinterpret_cast<qi::Future<void *> *>(fut);

  delete future;
}

void    qi_future_set_callback(qi_future_t *fut, QiFutureCallback cb, void *miscdata)
{
  qi::Future<void *> *future = reinterpret_cast<qi::Future<void *> *>(fut);
  FutureCallbackForwarder *forwarder = new FutureCallbackForwarder();

  forwarder->addCallback(cb);
  future->addCallbacks(forwarder, miscdata);
}

void    qi_future_wait(qi_future_t *fut)
{
  qi::Future<void *> *future = reinterpret_cast<qi::Future<void *> *>(fut);

  future->wait();
}

int     qi_future_is_error(qi_future_t *fut)
{
  qi::Future<void *> *future = reinterpret_cast<qi::Future<void *> *>(fut);

  return future->hasError();
}

int     qi_future_is_ready(qi_future_t *fut)
{
  qi::Future<void *> *future = reinterpret_cast<qi::Future<void *> *>(fut);

  return future->isReady();
}

qi_message_t *  qi_future_get_value(qi_future_t *fut)
{
  qi::Future<void *> *future = reinterpret_cast<qi::Future<void *> *>(fut);

  return (qi_message_t *) future->value();
}
