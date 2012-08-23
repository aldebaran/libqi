/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#include <qimessaging/object.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/message.hpp>

#include <qimessaging/c/object_c.h>
#include <qimessaging/c/message_c.h>
#include <qimessaging/c/future_c.h>

qi_buffer_t  *qi_message_get_buffer(qi_message_t *msg);

typedef struct
{
  qi::ODataStream *os;
  qi::IDataStream *is;
  qi::Message     *msg;
  qi::Buffer      *buff;
} qi_message_data_t;

class CFunctorResultBase : public qi::FunctorResultBase
{
public:
  CFunctorResultBase(const std::string &signature)
    : _signature(signature),
     _promise(new qi::Promise<void *>())
  {
  }
  ~CFunctorResultBase()
  {
    std::cout << "FunctorResultBase deleted" << std::endl;
  }

  inline virtual void setValue(const qi::Buffer &result)
  {
    if (_signature == "v") {
      _promise->setValue((void *) 0);
      return;
    }

    qi_message_data_t *new_message = (qi_message_data_t *) malloc(sizeof(qi_message_data_t));
    if (!new_message)
      _promise->setError("Cannot allocate memory to handle answer");
    else
    {
      memset(new_message, 0, sizeof(qi_message_data_t));
      new_message->buff = new qi::Buffer(result);
      _promise->setValue((void *) new_message);
    }
  }

  inline virtual void setError(const std::string &sig, const qi::Buffer &error) {
    qi::IDataStream ds(error);
    std::string err;
     if (sig != "s") {
      std::stringstream ss;
      ss << "Can't report the correct error message because the error signature is :" << sig;
      _promise->setError(ss.str());
      return;
    }
    ds >> err;
    _promise->setError(err);
  }

public:
  std::string               _signature;
  qi::Promise<void *>       *_promise;
};

class CFunctorResult : public qi::FunctorResult
{
public:
  CFunctorResult(boost::shared_ptr<CFunctorResultBase> base)
    : FunctorResult(base)
  {
    CFunctorResultBase *p = reinterpret_cast<CFunctorResultBase *>(_p.get());
    _future = p->_promise->future();
  }

  ~CFunctorResult()
  {
  }

qi::Future<void *> *future() { return &_future; }

private:
  qi::Future<void *>          _future;
};

qi_object_t *qi_object_create(const char *name)
{
  qi::Object *obj = new qi::Object();

  return (qi_object_t *) obj;
}

void        qi_object_destroy(qi_object_t *object)
{
  qi::Object *obj = reinterpret_cast<qi::Object *>(object);

  delete obj;
}

void         qi_object_connect(qi_object_t *object, const char *address)
{
  qi::Object *obj = reinterpret_cast<qi::Object *>(object);

  obj = 0; // what a feature
}

int          qi_object_get_object_id(qi_object_t *object)
{
  qi::Object *obj = reinterpret_cast<qi::Object *>(object);

  return 0;
}

int          qi_object_get_function_id(qi_object_t *object, const char *function_name)
{
  qi::Object *obj = reinterpret_cast<qi::Object *>(object);

  qi::MetaObject meta = obj->metaObject();

  return meta.methodId(function_name);
}

qi_future_t *qi_object_call(qi_object_t *object, const char *signature_c, qi_message_t *message)
{
  qi::Object *obj = reinterpret_cast<qi::Object *>(object);
  std::vector<std::string>  sigInfo;
  std::string fullSignature(signature_c);
  if (!obj || !signature_c || !message)
  {
    printf("Invalid parameter\n");
    return 0;
  }

  sigInfo = qi::signatureSplit(fullSignature);

  //get buffer from message
  qi::Buffer *buff = reinterpret_cast<qi::Buffer*>(qi_message_get_buffer(message));
  qi::FunctorParameters             request(*buff);
  boost::shared_ptr<CFunctorResultBase> base(new CFunctorResultBase(sigInfo[0]));
  CFunctorResult*                   promise = new CFunctorResult(base);

  fullSignature = sigInfo[1];
  fullSignature.append("::");
  fullSignature.append(sigInfo[2]);
  obj->xMetaCall(sigInfo[0], fullSignature, request, *promise);
  return (qi_future_t *) promise->future();
}
