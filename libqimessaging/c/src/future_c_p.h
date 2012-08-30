/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	_QIMESSAGING_FUTURE_C_P_H_
# define   	_QIMESSAGING_FUTURE_C_P_H_

#include <list>
class CFunctorResult;
class FutureCallbackForwarder;

typedef struct
{
  qi::Future<void *>                  *future;
  CFunctorResult                      *functor;
  std::list<FutureCallbackForwarder *> callbacks;
} qi_future_data_t;

//Needed to bridge C callback on C++ Future callbacks;
class FutureCallbackForwarder : public qi::FutureInterface<void *>
{
public:
  FutureCallbackForwarder() {}
  virtual ~FutureCallbackForwarder() {}

  virtual void onFutureFailed(const std::string &error, void *data)
  {
    std::list<qi_future_callback_t>::iterator it;

    for (it = _callbacks.begin(); it != _callbacks.end(); ++it)
    {
      (*(*it))(0, 0, data);
    }
  }

  virtual void onFutureFinished(void*const &value, void *data)
  {
    std::list<qi_future_callback_t>::iterator it;

    for (it = _callbacks.begin(); it != _callbacks.end(); ++it)
    {
      (*(*it))(value, 1, data);
    }
  }
  void addCallback(qi_future_callback_t callback)
  {
    _callbacks.push_back(callback);
  }

private:
  std::list<qi_future_callback_t>   _callbacks;
};

#endif
