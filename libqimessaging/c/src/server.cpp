/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#include <qimessaging/qi.h>
#include "src/messaging/server_impl.hpp"


qi_server_t *qi_server_create(const char *name) {
  qi::detail::ServerImpl *pserver = new qi::detail::ServerImpl(name);
  return reinterpret_cast<qi_server_t *>(pserver);
}

void         qi_server_destroy(qi_server_t *server) {
  qi::detail::ServerImpl  *pserver  = reinterpret_cast<qi::detail::ServerImpl *>(server);
  delete pserver;
}

void         qi_server_connect(qi_server_t *server, const char *address) {
  qi::detail::ServerImpl  *pserver  = reinterpret_cast<qi::detail::ServerImpl *>(server);
  pserver->connect(address);
}


class CFunctor : public qi::Functor {
public:
  CFunctor(const char *complete_sig, BoundMethod func, void *data = 0)
    : _func(func),
      _complete_sig(strdup(complete_sig)),
      _data(data)
  {
    ;
  }

  virtual void call(qi::DataStream &params, qi::DataStream& result)const {
    if (_func)
      _func(_complete_sig, reinterpret_cast<qi_message_t *>(&params), reinterpret_cast<qi_message_t *>(&result), _data);
  }

  virtual ~CFunctor() {
    free(_complete_sig);
  }

private:
  BoundMethod   _func;
  char         *_complete_sig;
  void         *_data;

};


void         qi_server_advertise_service(qi_server_t *server, const char *methodSignature, BoundMethod func, void *data) {
  qi::detail::ServerImpl  *pserver  = reinterpret_cast<qi::detail::ServerImpl *>(server);
  CFunctor *fun = new CFunctor(methodSignature, func, data);
  pserver->advertiseService(methodSignature, fun);
}

void         qi_server_unadvertise_service(qi_server_t *server, const char *methodSignature) {
  qi::detail::ServerImpl  *pserver  = reinterpret_cast<qi::detail::ServerImpl *>(server);
  pserver->unadvertiseService(methodSignature);
}
