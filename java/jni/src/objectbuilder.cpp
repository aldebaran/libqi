/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <map>
#include <qi/log.hpp>
#include <qitype/metamethod.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <jnitools.hpp>

#include <object_jni.hpp>
#include <callbridge.hpp>
#include <objectbuilder.hpp>

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderCreate(JNIEnv* env, jobject QI_UNUSED(jobj))
{
  // Keep a pointer on JavaVM singleton if not already set.
  JVM(env);

  qi::GenericObjectBuilder *ob = new qi::GenericObjectBuilder();
  return (jlong) ob;
}

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderGetObject(JNIEnv *env, jobject jobj, jlong pObjectBuilder)
{
  qi::GenericObjectBuilder *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  qi::ObjectPtr *obj = new qi::ObjectPtr();
  qi::ObjectPtr &o = *(reinterpret_cast<qi::ObjectPtr *>(obj));

  o = ob->object();
  return (jlong) obj;
}

void   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderDestroy(long pObjectBuilder)
{
  qi::GenericObjectBuilder *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  delete ob;
}

jlong   Java_com_aldebaran_qimessaging_GenericObjectBuilder_create()
{
  qi::GenericObjectBuilder *ob = new qi::GenericObjectBuilder();
  return (jlong) ob;
}

jobject Java_com_aldebaran_qimessaging_GenericObjectBuilder_object(JNIEnv* env, jobject QI_UNUSED(jobj), jlong pObjectBuilder)
{
  qi::GenericObjectBuilder *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  qi::ObjectPtr *obj = new qi::ObjectPtr();
  qi::ObjectPtr &o = *(reinterpret_cast<qi::ObjectPtr *>(obj));

  JVM(env);
  o = ob->object();

  JNIObject jobj(obj);
  return jobj.object();
}

void    Java_com_aldebaran_qimessaging_GenericObjectBuilder_destroy(JNIEnv *env, jobject jobj, jlong pObjectBuilder)
{
  qi::GenericObjectBuilder *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  delete ob;
}

jlong   Java_com_aldebaran_qimessaging_GenericObjectBuilder_advertiseMethod(JNIEnv *env, jobject jobj, jlong pObjectBuilder, jstring method, jobject instance, jstring className, jstring desc)
{
  extern MethodInfoHandler   gInfoHandler;
  qi::GenericObjectBuilder  *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  std::string                signature = qi::jni::toString(method);
  qi_method_info*            data;
  std::vector<std::string>   sigInfo;
  std::string                description = qi::jni::toString(desc);

  // Create a new global reference on object instance.
  // jobject structure are local reference and are destroyed when returning to JVM
  // Fixme : May leak global ref.
  instance = env->NewGlobalRef(instance);

  // Create a struct holding a jobject instance, jmethodId id and other needed thing for callback
  // Pass it to void * data to register_method
  // In java_callback, use it directly so we don't have to find method again
  data = new qi_method_info(instance, signature, jobj, qi::jni::toString(className));
  gInfoHandler.push(data);

  // Bind method signature on generic java callback
  sigInfo = qi::signatureSplit(signature);
  int ret = ob->xAdvertiseMethod(sigInfo[0],
                                 sigInfo[1],
                                 sigInfo[2],
                                 makeDynamicGenericFunction(boost::bind(&call_to_java, signature, data, _1)).dropFirstArgument(),
                                 description);

  return (jlong) ret;
}

jlong   Java_com_aldebaran_qimessaging_GenericObjectBuilder_advertiseSignal(JNIEnv *env, jobject obj, jlong pObjectBuilder, jstring eventSignature)
{
  qi::GenericObjectBuilder  *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  std::vector<std::string>   sigInfo = qi::signatureSplit(qi::jni::toString(eventSignature));
  std::string   event = sigInfo[1];
  std::string   callbackSignature = sigInfo[0] + sigInfo[2];

  return (jlong) ob->xAdvertiseSignal(event, callbackSignature);
}

jlong   Java_com_aldebaran_qimessaging_GenericObjectBuilder_advertiseProperty(JNIEnv *env, jobject QI_UNUSED(obj), jlong pObjectBuilder, jstring jname, jclass propertyBase)
{
  qi::GenericObjectBuilder  *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  std::string                name = qi::jni::toString(jname);

  std::string sig = propertyBaseSignature(env, propertyBase);
  return (jlong) ob->xAdvertiseProperty(name, sig);
}
