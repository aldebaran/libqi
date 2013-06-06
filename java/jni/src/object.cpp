/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#include <qitype/genericobject.hpp>

#include <jnitools.hpp>
#include <object.hpp>
#include <callbridge.hpp>
#include <jobjectconverter.hpp>

qiLogCategory("qimessaging.jni");

extern MethodInfoHandler gInfoHandler;

jlong   Java_com_aldebaran_qimessaging_Object_property(JNIEnv* env, jobject jobj, jlong pObj, jstring name)
{
  qi::ObjectPtr&     obj = *(reinterpret_cast<qi::ObjectPtr*>(pObj));
  std::string        propName = toStdString(env, name);

  qi::Future<qi::GenericValue>* ret = new qi::Future<qi::GenericValue>();

  JVM(env);
  JVM()->AttachCurrentThread((envPtr) &env, (void*) 0);

  try
  {
    *ret = obj->property<qi::GenericValue>(propName).async();
  } catch (qi::FutureUserException& e)
  {
    throwJavaError(env, e.what());
    return 0;
  }

  return (jlong) ret;
}

jlong  Java_com_aldebaran_qimessaging_Object_setProperty(JNIEnv* env, jobject QI_UNUSED(jobj), jlong pObj, jstring name, jobject property)
{
  qi::ObjectPtr&    obj = *(reinterpret_cast<qi::ObjectPtr*>(pObj));
  std::string        propName = toStdString(env, name);

  JVM(env);
  JVM()->AttachCurrentThread((envPtr) &env, (void*) 0);

  qi::Future<void>* ret = new qi::Future<void>();
  *ret = obj->setProperty(propName, qi::GenericValue::from<jobject>(property)).async();

  return (jlong) ret;
}

jlong     Java_com_aldebaran_qimessaging_Object_asyncCall(JNIEnv* env, jobject QI_UNUSED(jobj), jlong pObject, jstring jmethod, jobjectArray args)
{
  qi::ObjectPtr&    obj = *(reinterpret_cast<qi::ObjectPtr*>(pObject));
  std::string       method;
  qi::Future<qi::GenericValuePtr>* fut = 0;

  // Init JVM singleton and attach current thread to JVM
  JVM(env);
  JVM()->AttachCurrentThread((envPtr) &env, (void*) 0);

  if (!obj)
  {
    qiLogError() << "Given object not valid.";
    throwJavaError(env, "Given object is not valid.");
    return 0;
  }

  // Get method name and parameters C style.
  method = toStdString(env, jmethod);
  try {
    fut = call_from_java(env, obj, method, args);
  } catch (std::exception& e)
  {
    throwJavaError(env, e.what());
    return 0;
  }

  return (jlong) fut;
}

jstring   Java_com_aldebaran_qimessaging_Object_printMetaObject(JNIEnv* env, jobject QI_UNUSED(jobj), jlong pObject)
{
  qi::ObjectPtr&    obj = *(reinterpret_cast<qi::ObjectPtr*>(pObject));
  std::stringstream ss;

  qi::details::printMetaObject(ss, obj->metaObject());
  return toJavaString(env, ss.str());
}

void      Java_com_aldebaran_qimessaging_Object_destroy(JNIEnv* QI_UNUSED(env), jobject QI_UNUSED(jobj), jlong pObject)
{
  qi::ObjectPtr*    obj = reinterpret_cast<qi::ObjectPtr*>(pObject);

  delete obj;
}

jlong     Java_com_aldebaran_qimessaging_Object_connect(JNIEnv *env, jobject jobj, jlong pObject, jstring method, jobject instance, jstring service, jstring eventName)
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

  // Remove return value
  sigInfo = qi::signatureSplit(signature);
  signature = sigInfo[1];
  signature.append("::");
  signature.append(sigInfo[2]);

  // Create a struct holding a jobject instance, jmethodId id and other needed thing for callback
  // Pass it to void * data to register_method
  data = new qi_method_info(instance, signature, jobj, toStdString(env, service));
  gInfoHandler.push(data);


  return obj->connect(event,
                       qi::SignalSubscriber(
                         qi::makeDynamicGenericFunction(
                           boost::bind(&event_callback_to_java, (void*) data, _1)),
                         qi::MetaCallType_Direct));
}

void      Java_com_aldebaran_qimessaging_Object_post(JNIEnv *env, jobject QI_UNUSED(jobj), jlong pObject, jstring eventName, jobjectArray jargs)
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
    qiLogError() << "Cannot attach callback thread to Java VM";
    throwJavaError(env, "Cannot attach callback thread to Java VM");
    return;
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
    signature += params[i].signature(true).toString();
  signature += ")";

  obj->metaPost(event, params);

  // Destroy arguments
  i = 0;
  for(qi::GenericFunctionParameters::iterator it = params.begin(); it != params.end(); ++it)
    (*it).destroy();
  return;
}
