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
  qi::FunctorParameters             request(*m->buff);
  boost::shared_ptr<CFunctorResultBase> base(new CFunctorResultBase(sigInfo[0]));
  CFunctorResult*                   promise = new CFunctorResult(base);

  fullSignature = sigInfo[1];
  fullSignature.append("::");
  fullSignature.append(sigInfo[2]);
  obj->xMetaCall(sigInfo[0], fullSignature, request, *promise);

  qi_future_data_t*     data = new qi_future_data_t;

  data->functor = promise;
  data->future = promise->future();
  return (qi_future_t *) data;
}

int          qi_object_register_method(qi_object_t *object, const char *complete_signature, qi_object_method_t func, void *data)
{
  qi::Object *obj = reinterpret_cast<qi::Object *>(object);
  std::string signature(complete_signature);
  std::vector<std::string>  sigInfo;

  sigInfo = qi::signatureSplit(signature);
  qi::Functor* functor = new CFunctor(complete_signature, func, data);
  signature = sigInfo[1];
  signature.append("::");
  signature.append(sigInfo[2]);
  obj->xAdvertiseMethod(sigInfo[0], signature, functor);
  return 0;
}
