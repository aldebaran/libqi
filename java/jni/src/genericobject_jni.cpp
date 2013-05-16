/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qi/future.hpp>

#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/functiontype.hpp>

#include "genericobject_jni.hpp"
#include "jobjectconverter.hpp"
#include "jnitools.hpp"

static MethodInfoHandler gInfoHandler;

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectCreate()
{
  qi::ObjectPtr *obj = new qi::ObjectPtr();
  return (jlong) obj;
}

void    Java_com_aldebaran_qimessaging_GenericObject_qiObjectDestroy(JNIEnv *env, jobject jobj, jlong pObject)
{
  qi::ObjectPtr *obj = reinterpret_cast<qi::ObjectPtr *>(pObject);
  gInfoHandler.pop(jobj);

  delete obj;
}

static qi::Future<qi::GenericValuePtr>* async_call_java
(JNIEnv *env, qi::ObjectPtr object, const std::string& strMethodName, jobjectArray listParams)
{
  qi::GenericFunctionParameters params;
  std::string signature;
  jsize size;
  jsize i = 0;
  bool  useSignature = true;

  size = env->GetArrayLength(listParams);
  while (i < size)
  {
    jobject current = env->NewGlobalRef(env->GetObjectArrayElement(listParams, i));
    qi::GenericValuePtr val = qi::GenericValueRef(current).clone();
    params.push_back(val);

    // In case of empty list or map, type system cannot resole containee type. In that case, do not use signature.
    if (val.signature(true).find(qi::Signature::Type_None) != std::string::npos)
      useSignature = false;

    signature += val.signature(true);
    i++;
  }

  // If user provide signature, do not use resolved one.
  if (strMethodName.find("::") != std::string::npos)
    useSignature = false;

  qi::Future<qi::GenericValuePtr> *fut = new qi::Future<qi::GenericValuePtr>();
  try
  {
    std::string nameWithOptionalSignature = strMethodName + (useSignature == true ? "::(" + signature + ")" : "");
    *fut = object->metaCall(nameWithOptionalSignature, params);
  } catch (std::runtime_error &e)
  {
    throwJavaError(env, e.what());
    return 0;
  }

  return fut;
}

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectAsyncCall
(JNIEnv *env, jobject QI_UNUSED(jobj), jlong pObject, jstring jmethod, jobjectArray args)
{
  qi::ObjectPtr obj = *(reinterpret_cast<qi::ObjectPtr *>(pObject));
  std::string  method;

  // Init JVM singleton and attach current thread to JVM
  JVM(env);
  JVM()->AttachCurrentThread((envPtr) &env, (void*) 0);

  if (!obj)
  {
    qiLogError("qimessaging.java") << "Given object not valid.";
    throwJavaError(env, "Given object is not valid.");
    return 0;
  }

  // Get method name and parameters C style.
  method = toStdString(env, jmethod);
  qi::Future<qi::GenericValuePtr> *fut = async_call_java(env, obj, method, args);
  return (jlong) fut;
}

static jobject qi_generic_call_java(JNIEnv *env, qi::ObjectPtr object, const std::string& strMethodName, jobjectArray listParams)
{
  qi::Future<qi::GenericValuePtr> *fut = async_call_java(env, object, strMethodName, listParams);

  if(fut == 0)
  {
    throwJavaError(env, "Future value not inialized");
    return 0;
  }

  fut->wait();
  if (fut->hasError())
  {
    throwJavaError(env, fut->error().c_str());
    return 0;
  }

  jobject ret = fut->value().to<jobject>();
  delete fut;
  return ret;
}

jobject Java_com_aldebaran_qimessaging_GenericObject_qiObjectCall
(JNIEnv *env, jobject QI_UNUSED(jobj), jlong pObject, jstring jmethod, jobjectArray args)
{
  qi::ObjectPtr obj = *(reinterpret_cast<qi::ObjectPtr *>(pObject));
  std::string  method;

  // Init JVM singleton and attach current thread to JVM
  JVM(env);
  JVM()->AttachCurrentThread((envPtr) &env, (void*) 0);

  if (!obj)
  {
    qiLogError("qimessaging.java") << "Given object not valid.";
    throwJavaError(env, "Given object is not valid.");
    return 0;
  }

  // Get method name and parameters C style.
  method = toStdString(env, jmethod);
  return qi_generic_call_java(env, obj, method, args);
}

qi::GenericValuePtr java_call
(std::string signature, void* data, const qi::GenericFunctionParameters& params)
{
  qi::GenericValuePtr res;
  jvalue*             args = new jvalue[params.size()];
  int                 index = 0;
  JNIEnv*             env = 0;
  qi_method_info*     info = reinterpret_cast<qi_method_info*>(data);
  std::vector<std::string>  sigInfo = qi::signatureSplit(signature);

  // Attach JNIEnv to current thread to avoid segfault in jni functions. (eventloop dependent)
  if (JVM()->AttachCurrentThread((envPtr) &env, (void *) 0) != JNI_OK || env == 0)
  {
    qiLogError("qimessaging.java") << "Cannot attach callback thread to Java VM";
    throwJavaError(env, "Cannot attach callback thread to Java VM");
    return res;
  }

  // Check value of method info structure
  if (info == 0)
  {
    qiLogError("qimessaging.java") << "Internal method informations are not valid";
    throwJavaError(env, "Internal method informations are not valid");
    return res;
  }
  // Translate parameters from GenericValues to jobjects
  qi::GenericFunctionParameters::const_iterator it = params.begin();
  qi::GenericFunctionParameters::const_iterator end = params.end();
  for(; it != end; it++)
  {
    jvalue value;
    value.l = JObject_from_GenericValue(*it);
    args[index] = value;
    index++;
  }

  // Finally call method
  // If method is declared as static, it won't work, doc that.
  jclass cls = env->FindClass(info->className.c_str());
  jmethodID mid = env->GetMethodID(cls, sigInfo[1].c_str(), toJavaSignature(signature).c_str());
  if (!mid)
  {
    qiLogError("qimessaging.jni") << "Cannot find java method " << sigInfo[1] << toJavaSignature(signature).c_str();
    throw new std::runtime_error("Cannot find method");
  }

  jobject ret = env->CallObjectMethodA(info->instance, mid, args);

  delete[] args;
  if (sigInfo[0] == "")
    return qi::GenericValuePtr(qi::typeOf<void>());

  return GenericValue_from_JObject(ret).first;
}

jlong Java_com_aldebaran_qimessaging_GenericObject_qiObjectRegisterMethod
(JNIEnv *env, jobject jobj, jlong pObjectBuilder, jstring method, jobject instance, jstring className /*, jstring description*/)
{
  qi::GenericObjectBuilder  *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  std::string                signature = toStdString(env, method);
  qi_method_info*            data;
  std::vector<std::string>  sigInfo;

  // Keep a pointer on JavaVM singleton if not already set.
  JVM(env);

  // Create a new global reference on object instance.
  // jobject structure are local reference and are destroyed when returning to JVM
  // Fixme : May leak global ref.
  instance = env->NewGlobalRef(instance);

  // Create a struct holding a jobject instance, jmethodId id and other needed thing for callback
  // Pass it to void * data to register_method
  // In java_callback, use it directly so we don't have to find method again
  data = new qi_method_info(instance, signature, jobj, toStdString(env, className));
  gInfoHandler.push(data);

  // Bind method signature on generic java callback
  sigInfo = qi::signatureSplit(signature);
  int ret = ob->xAdvertiseMethod(sigInfo[0],
                                 sigInfo[1],
                                 sigInfo[2],
                                 makeDynamicGenericFunction(boost::bind(&java_call, signature, data, _1)).dropFirstArgument(),
                                 std::string(""));

  return (jlong) ret;
}

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectAdvertiseSignal
(JNIEnv *env, jobject QI_UNUSED(obj), jlong pObjectBuilder, jstring eventSignature)
{
  qi::GenericObjectBuilder  *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  std::vector<std::string>   sigInfo = qi::signatureSplit(toStdString(env, eventSignature));
  std::string   event = sigInfo[1];
  std::string   callbackSignature = sigInfo[0] + sigInfo[2];

  // Keep a pointer on JavaVM singleton if not already set.
  JVM(env);

  //I.E : jlong ret = (jlong) ob->advertiseEvent<void (*)(const int&)>(event);
  jlong ret = (jlong) ob->xAdvertiseSignal(event, callbackSignature);
  return ret;
}

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectPost
(JNIEnv *env, jobject QI_UNUSED(callingObj), jlong pObject, jstring eventName, jobjectArray jargs)
{
  qi::ObjectPtr obj = *(reinterpret_cast<qi::ObjectPtr *>(pObject));
  std::string   event = toStdString(env, eventName);
  qi::GenericFunctionParameters params;
  std::string signature;
  jsize size;
  jsize i = 0;

  // Attach JNIEnv to current thread to avoid segfault in jni functions. (eventloop dependent)
  if (JVM()->AttachCurrentThread((envPtr) &env, (void *) 0) != JNI_OK || env == 0)
  {
    qiLogError("qimessaging.java") << "Cannot attach callback thread to Java VM";
    throwJavaError(env, "Cannot attach callback thread to Java VM");
    return 0;
  }

  size = env->GetArrayLength(jargs);
  i = 0;
  while (i < size)
  {
    jobject current = env->NewGlobalRef(env->GetObjectArrayElement(jargs, i));
    qi::GenericValuePtr val = qi::GenericValueRef(GenericValue_from_JObject(current).first);
    params.push_back(val);
    i++;
  }

  // Signature construction
  signature = event + "::(";
  for (unsigned i=0; i< params.size(); ++i)
    signature += params[i].signature(true);
  signature += ")";

  obj->metaPost(signature, params);
  return 0;
}

qi::GenericValuePtr java_event_callback
(void *vinfo, const std::vector<qi::GenericValuePtr>& params)
{
  qi_method_info*  info = static_cast<qi_method_info*>(vinfo);
  qiLogVerbose("qimessaging.jni") << "Java event callback called (sig=" << info->sig << ")";
  return java_call(info->sig, info, params);
}

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectConnect
(JNIEnv *env, jobject jobj, jlong pObject, jstring method, jobject instance, jstring service, jstring eventName)
{
  qi::ObjectPtr&             obj = *(reinterpret_cast<qi::ObjectPtr *>(pObject));
  std::string                signature = toStdString(env, method);
  std::string                event = toStdString(env, eventName);
  qi_method_info*            data;
  std::vector<std::string>  sigInfo;

  // Keep a pointer on JavaVM singleton if not already set.
  JVM(env);

  // Create a new global reference on object instance.
  // jobject structure are local reference and are destroyed when returning to JVM
  // Fixme : May leak global ref.
  instance = env->NewGlobalRef(instance);

  // Create a struct holding a jobject instance, jmethodId id and other needed thing for callback
  // Pass it to void * data to register_method
  data = new qi_method_info(instance, signature + "::(i)", jobj, toStdString(env, service));
  gInfoHandler.push(data);

  // Bind method signature on generic java callback
  sigInfo = qi::signatureSplit(signature);
  signature = sigInfo[1];
  signature.append("::");
  signature.append(sigInfo[2]);

  return obj->xConnect(event + "::" + "(i)",
                  qi::SignalSubscriber(
                         qi::makeDynamicGenericFunction(
                           boost::bind(&java_event_callback, (void*) data, _1)),
                           qi::MetaCallType_Direct));
}
