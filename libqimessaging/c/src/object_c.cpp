/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/

#include <qitype/genericobject.hpp>
#include <qitype/genericvalue.hpp>
#include <qimessaging/datastream.hpp>
#include <qimessaging/message.hpp>
#include <qitype/genericobjectbuilder.hpp>

#include <qimessaging/c/object_c.h>
#include <qimessaging/c/message_c.h>
#include <qimessaging/c/future_c.h>
#include "message_c_p.h"
#include "object_c_p.h"
#include "future_c_p.h"

void qiFutureCAdapter(qi::Future<qi::GenericValue> result, qi::Promise<void*> promise) {
  if (result.hasError()) {
    promise.setError(result.error());
    return;
  }
  qi_message_t* msg = qi_message_create();
  qi_message_data_t* msgData = (qi_message_data_t*)msg;
  qi::Buffer buf;
  qi::ODataStream out(buf);
  result.value().serialize(out);
  *msgData->buff = buf;
  promise.setValue(msg);
}

qi_object_t *qi_object_create()
{
  qi::ObjectPtr *obj = new qi::ObjectPtr();
  return (qi_object_t *) obj;
}

void        qi_object_destroy(qi_object_t *object)
{
  qi::ObjectPtr *obj = reinterpret_cast<qi::ObjectPtr *>(object);

  delete obj;
}

qi_future_t *qi_object_call(qi_object_t *object, const char *signature_c, qi_message_t *message)
{
  qi::ObjectPtr obj = *(reinterpret_cast<qi::ObjectPtr *>(object));

  // Get sigreturn for functor result
  int methodId = obj->metaObject().methodId(signature_c);
  const qi::MetaMethod *mm = obj->metaObject().method(methodId);

  // Get buffer from message
  qi_message_data_t *m = reinterpret_cast<qi_message_data_t*>(message);

  std::string sig = mm->signature();
  sig = qi::signatureSplit(sig)[2];
  sig = sig.substr(1, sig.length() - 2);
  qi::GenericFunctionParameters params;
  params = qi::GenericFunctionParameters::fromBuffer(sig, *m->buff);
  qi::Future<qi::GenericValue> res = obj->xMetaCall(mm->sigreturn(), signature_c, params);
  qi::Promise<void*> promise;
  qi_future_data_t*  data = new qi_future_data_t;
  res.connect(boost::bind<void>(&qiFutureCAdapter, _1, promise));
  data->future = new qi::Future<void*>();
  *data->future = promise.future();
  return (qi_future_t *) data;
}

// ObjectBuilder

qi_object_builder_t *qi_object_builder_create()
{
  qi::GenericObjectBuilder *ob = new qi::GenericObjectBuilder();
  return (qi_object_builder_t *) ob;
}

void        qi_object_builder_destroy(qi_object_builder_t *object_builder)
{
  qi::GenericObjectBuilder *ob = reinterpret_cast<qi::GenericObjectBuilder *>(object_builder);
  delete ob;
}

qi::GenericValue c_call(
  std::string complete_sig,
  qi_object_method_t func,
  void* data,
  const qi::GenericFunctionParameters& params)
{
  qi_message_data_t* message_c = (qi_message_data_t *) malloc(sizeof(qi_message_data_t));
  qi_message_data_t* answer_c = (qi_message_data_t *) malloc(sizeof(qi_message_data_t));

   memset(message_c, 0, sizeof(qi_message_data_t));
   memset(answer_c, 0, sizeof(qi_message_data_t));

   message_c->buff = new qi::Buffer(params.toBuffer());
   answer_c->buff = new qi::Buffer();

   if (func)
     func(complete_sig.c_str(), (qi_message_t *) message_c, reinterpret_cast<qi_message_t *>(answer_c), data);

   qi::GenericValue res;
   std::string sigret = qi::signatureSplit(complete_sig)[0];
   res.type = qi::Type::fromSignature(sigret);
   qi::IDataStream in(*answer_c->buff);
   res = res.type->deserialize(in);
   qi_message_destroy((qi_message_t *) message_c);
   qi_message_destroy((qi_message_t *) answer_c);
   return res;
}


int          qi_object_builder_register_method(qi_object_builder_t *object_builder, const char *complete_signature, qi_object_method_t func, void *data)
{
  qi::GenericObjectBuilder  *ob = reinterpret_cast<qi::GenericObjectBuilder *>(object_builder);
  std::string signature(complete_signature);
  std::vector<std::string>  sigInfo;

  sigInfo = qi::signatureSplit(signature);
  signature = sigInfo[1];
  signature.append("::");
  signature.append(sigInfo[2]);
  ob->xAdvertiseMethod(sigInfo[0], signature,
    makeDynamicGenericFunction(
    boost::bind(&c_call, std::string(complete_signature), func, data, _1)));
  return 0;
}

qi_object_t*         qi_object_builder_get_object(qi_object_builder_t *object_builder) {
  qi::GenericObjectBuilder *ob = reinterpret_cast<qi::GenericObjectBuilder *>(object_builder);
  qi_object_t *obj = qi_object_create();
  qi::ObjectPtr &o = *(reinterpret_cast<qi::ObjectPtr *>(obj));

  o = ob->object();
  return obj;
}
