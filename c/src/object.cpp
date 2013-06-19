/*
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebararan Robotics
*/
#include <qic/object.h>
#include <qic/future.h>
#include <qic/value.h>
#include "future_p.h"
#include "object_p.h"
#include "value_p.h"

#include <qitype/genericobject.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/genericobjectbuilder.hpp>



qiLogCategory("qimessaging.object");


static qi::AnyReference c_call(const std::string &complete_sig,
                                          qi_object_method_t func,
                                              void* data,
                                  const qi::GenericFunctionParameters& params)
{
  //TODO: move to register
  std::vector<std::string> vs = qi::signatureSplit(std::string(complete_sig));
  qi_value_t* value = qi_value_create(vs[2].c_str());
  qi_value_t* ret = qi_value_create(vs[0].c_str());
  // First argument is the DynamicObject instance, drop it.
  qi::GenericFunctionParameters remove_first;
  if (params.size() > 1)
    remove_first.insert(remove_first.end(), params.begin()+1, params.end());
  qi::GenericValue &gvp = qi_value_cpp(value);
  //TODO: there is a copy here
  gvp = qi::GenericValue::makeTuple(remove_first);
  std::cout << "Complete sig:" << complete_sig << std::endl;

  if (func)
    func(complete_sig.c_str(), value, ret, data);

  qi::AnyReference &gvpr = qi_value_cpp(ret);
  qi::AnyReference re = gvpr;
  //just reset the gvp, we dont want destroy to destroy it...
  gvpr.type = 0;
  gvpr.value = 0;
  qi_value_destroy(value);
  qi_value_destroy(ret);
  return re;
}

static qi::AnyReference c_signal_callback(const std::vector<qi::AnyReference>& args,
                                             const std::string &params_sigs,
                                             qi_object_signal_callback_t f,
                                             void *user_data) {
  qiLogInfo() << "Signal Callback(" << params_sigs << ")";

  qi_value_t* params = qi_value_create(params_sigs.c_str());
  qi::GenericValue &gvp = qi_value_cpp(params);
  gvp = qi::GenericValue::makeTuple(args);
  f(params, user_data);
  qi_value_destroy(params);
  return qi::AnyReference();
}



#ifdef __cplusplus
extern "C"
{
#endif

void qiFutureCAdapter(qi::Future<qi::AnyReference> result, qi::Promise<qi::GenericValue> promise) {
  if (result.hasError()) {
    promise.setError(result.error());
    return;
  }
  qi::GenericValue gv;
  //we take the ownership of the GVP content. we are now responsible for freeing it.
  gv.reset(result.value(), false, true);
  promise.setValue(gv);
}

qi_object_t *qi_object_create()
{
  qi::ObjectPtr *obj = new qi::ObjectPtr();
  return (qi_object_t *) obj;
}

void        qi_object_destroy(qi_object_t *object)
{
  qi::ObjectPtr *obj = &qi_object_cpp(object);
  delete obj;
}

qi_future_t *qi_object_call(qi_object_t *object, const char *signature_c, qi_value_t *params)
{
  qi::ObjectPtr             &obj = qi_object_cpp(object);
  qi::GenericValue           gv  = qi_value_cpp(params);

  qi::Future<qi::AnyReference> res = obj->metaCall(signature_c, gv.asTupleValuePtr());
  qi::Promise<qi::GenericValue> prom;
  res.connect(boost::bind<void>(&qiFutureCAdapter, _1, prom));
  return qi_cpp_promise_get_future(prom);
}

// ObjectBuilder

qi_object_builder_t *qi_object_builder_create()
{
  qi::DynamicObjectBuilder *ob = new qi::DynamicObjectBuilder();
  return (qi_object_builder_t *) ob;
}

void        qi_object_builder_destroy(qi_object_builder_t *object_builder)
{
  qi::DynamicObjectBuilder *ob = reinterpret_cast<qi::DynamicObjectBuilder *>(object_builder);
  delete ob;
}

qi_value_t*          qi_object_get_metaobject(qi_object_t *object)
{
  qi::ObjectPtr &obj = *(reinterpret_cast<qi::ObjectPtr *>(object));
  const qi::MetaObject &mo = obj->metaObject();
  qi_value_t *ret = qi_value_create("");

  qi_value_cpp(ret) = qi::GenericValue::from(mo);
  return ret;
}

int                 qi_object_event_emit(qi_object_t* object, const char *signature, qi_value_t* params) {
  qi::ObjectPtr       &obj = qi_object_cpp(object);
  qi::AnyReference &val = qi_value_cpp(params);
  if (qi_value_get_kind(params) != QI_VALUE_KIND_TUPLE)
    return -1;
  obj->metaPost(signature, val.asTupleValuePtr());
  return 0;
}



qi_future_t*        qi_object_event_connect(qi_object_t* object, const char *signature, qi_object_signal_callback_t f, void* user_data) {
  qi::ObjectPtr &obj = qi_object_cpp(object);
  std::vector<std::string> vs = qi::signatureSplit(std::string(signature));
  qi::DynamicFunction fn = boost::bind<qi::AnyReference>(&c_signal_callback, _1, vs[2], f, user_data);
  return qi_future_wrap(obj->connect(signature, qi::AnyFunction::fromDynamicFunction(fn)));
}

qi_future_t*        qi_object_event_disconnect(qi_object_t* object, unsigned long long id) {
  qi::ObjectPtr &obj = qi_object_cpp(object);
  return qi_future_wrap(obj->disconnect(id));
}

int          qi_object_builder_register_method(qi_object_builder_t *object_builder, const char *complete_signature, qi_object_method_t func, void *data)
{
  qi::DynamicObjectBuilder  *ob = reinterpret_cast<qi::DynamicObjectBuilder *>(object_builder);
  std::string signature(complete_signature);
  std::vector<std::string>  sigInfo;

  sigInfo = qi::signatureSplit(signature);
  ob->xAdvertiseMethod(sigInfo[0], sigInfo[1], sigInfo[2],
    qi::AnyFunction::fromDynamicFunction(
    boost::bind(&c_call, std::string(complete_signature), func, data, _1)));
  return 0;
}

int          qi_object_builder_register_event(qi_object_builder_t *object_builder, const char *complete_signature)
{
  qi::DynamicObjectBuilder  *ob = reinterpret_cast<qi::DynamicObjectBuilder *>(object_builder);
  std::vector<std::string>  sigInfo;
  sigInfo = qi::signatureSplit(complete_signature);
  return ob->xAdvertiseSignal(sigInfo[1], sigInfo[2]);
}

int          qi_object_builder_register_property(qi_object_builder_t *object_builder, const char *complete_signature)
{
  qi::DynamicObjectBuilder  *ob = reinterpret_cast<qi::DynamicObjectBuilder *>(object_builder);
  std::vector<std::string>  sigInfo;
  sigInfo = qi::signatureSplit(complete_signature);
  return ob->xAdvertiseProperty(sigInfo[1], sigInfo[2]);
}

qi_object_t*         qi_object_builder_get_object(qi_object_builder_t *object_builder) {
  qi::DynamicObjectBuilder *ob = reinterpret_cast<qi::DynamicObjectBuilder *>(object_builder);
  qi_object_t *obj = qi_object_create();
  qi::ObjectPtr &o = *(reinterpret_cast<qi::ObjectPtr *>(obj));

  o = ob->object();
  return obj;
}

#ifdef __cplusplus
}
#endif

