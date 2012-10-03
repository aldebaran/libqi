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
#include <qimessaging/type.hpp>
#include <qimessaging/future.hpp>

class CFunctorResult;
class FutureCallbackForwarder;

typedef struct
{
  qi::Future<void*>                   *future;
  std::list<FutureCallbackForwarder *> callbacks;
} qi_future_data_t;

//Needed to bridge C callback on C++ Future callbacks;
class FutureCallbackForwarder
{
public:
  FutureCallbackForwarder() {}
  virtual ~FutureCallbackForwarder() {}

  virtual void onResult(qi::Future<void *> value, void *data)
  {
    std::list<qi_future_callback_t>::iterator it;

    if (value.hasError())
    {
      for (it = _callbacks.begin(); it != _callbacks.end(); ++it) {
        (*(*it))(0, 0, data);
      }
      return;
    }
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
