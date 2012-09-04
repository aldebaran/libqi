/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#include <qimessaging/object.hpp>
#include <qimessaging/metavalue.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/message.hpp>

#include <qimessaging/c/object_c.h>
#include <qimessaging/c/message_c.h>
#include <qimessaging/c/future_c.h>
#include "message_c_p.h"
#include "object_c_p.h"
#include "future_c_p.h"


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

class QiFutureCAdapter: public qi::FutureInterface<qi::MetaFunctionResult>
{
public:
  QiFutureCAdapter(qi::Promise<void*>& prom)
  : promise(prom)
  {}
  virtual void onFutureFinished(const qi::MetaFunctionResult &result, void *data)
  {
    qi_message_t* msg = qi_message_create();
    qi_message_data_t* msgData = (qi_message_data_t*)msg;
    *msgData->buff = result.getBuffer();
    promise.setValue(msg);
    delete this;
  }
  virtual void onFutureFailed(const std::string &error, void *data)
  {
    promise.setError(error);
    delete this;
  }
  qi::Promise<void*> promise;
};

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
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(message);

  fullSignature = sigInfo[1];
  fullSignature.append("::");
  fullSignature.append(sigInfo[2]);
  qi::Future<qi::MetaFunctionResult> res = obj->xMetaCall(sigInfo[0], fullSignature, qi::MetaFunctionParameters(*m->buff));
  qi::Promise<void*> promise;
  qi_future_data_t*  data = new qi_future_data_t;
  res.addCallbacks(new QiFutureCAdapter(promise));
  data->future = new qi::Future<void*>();
  *data->future = promise.future();
  return (qi_future_t *) data;
}

qi::MetaFunctionResult c_call(
  std::string complete_sig,
  qi_object_method_t func,
  void* data,
  const qi::MetaFunctionParameters& params)
{
  qi_message_data_t* message_c = (qi_message_data_t *) malloc(sizeof(qi_message_data_t));
    qi_message_data_t* answer_c = (qi_message_data_t *) malloc(sizeof(qi_message_data_t));

    memset(message_c, 0, sizeof(qi_message_data_t));
    memset(answer_c, 0, sizeof(qi_message_data_t));

    message_c->buff = new qi::Buffer(params.getBuffer());
    answer_c->buff = new qi::Buffer();

    if (func)
      func(complete_sig.c_str(), (qi_message_t *) message_c, reinterpret_cast<qi_message_t *>(answer_c), data);

    qi::MetaFunctionResult res(*answer_c->buff);
    // FIXME dog
    //result.setValue(*answer_c->buff);
    qi_message_destroy((qi_message_t *) message_c);
    qi_message_destroy((qi_message_t *) answer_c);
    return res;
}



int          qi_object_register_method(qi_object_t *object, const char *complete_signature, qi_object_method_t func, void *data)
{
  qi::Object *obj = reinterpret_cast<qi::Object *>(object);
  std::string signature(complete_signature);
  std::vector<std::string>  sigInfo;

  sigInfo = qi::signatureSplit(signature);
  signature = sigInfo[1];
  signature.append("::");
  signature.append(sigInfo[2]);
  obj->xAdvertiseMethod(sigInfo[0], signature,
    boost::bind(&c_call, std::string(complete_signature), func, data, _1));
  return 0;
}
