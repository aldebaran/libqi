/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <stdexcept>

#include <qi/log.hpp>
#include <qi/api.hpp>
#include <qimessaging/session.hpp>

#include <jni.h>
#include <jnitools.hpp>
#include <session_jni.hpp>
#include <object_jni.hpp>

jlong Java_com_aldebaran_qimessaging_Session_qiSessionCreate()
{
  qi::Session *session = new qi::Session();
  return (jlong) session;
}

void Java_com_aldebaran_qimessaging_Session_qiSessionDestroy(jlong pSession)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(pSession);
  delete s;
}

jboolean Java_com_aldebaran_qimessaging_Session_qiSessionIsConnected(JNIEnv* QI_UNUSED(env), jobject QI_UNUSED(obj), jlong pSession)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(pSession);

  return (jboolean) s->isConnected();
}

jlong Java_com_aldebaran_qimessaging_Session_qiSessionConnect(JNIEnv *env, jobject QI_UNUSED(obj), jlong pSession, jstring jurl)
{
  if (pSession == 0)
  {
    throwJavaError(env, "Given qi::Session doesn't exists (pointer null).");
    return 0;
  }

  // After this function return, callbacks are going to be set on new created Future.
  // Save the JVM pointer here to avoid big issues when callbacks will be called.
  JVM(env);

  qi::Session *s = reinterpret_cast<qi::Session*>(pSession);
  qi::Future<void> *fut = new qi::Future<void>();
  *fut = s->connect(toStdString(env, jurl).c_str()).async();

  if (fut->hasError())
  {
    qiLogError("qimessaging.jni") << "Error : " << fut->error();
    throwJavaError(env, fut->error().c_str());
  }

  return (jlong) fut;
}

void Java_com_aldebaran_qimessaging_Session_qiSessionClose(JNIEnv* QI_UNUSED(env), jobject QI_UNUSED(obj), jlong pSession)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(pSession);

  s->close();
}

jobject   Java_com_aldebaran_qimessaging_Session_service(JNIEnv* env, jobject QI_UNUSED(obj), jlong pSession, jstring jname)
{
  qi::Session *s = reinterpret_cast<qi::Session*>(pSession);
  std::string serviceName = toStdString(env, jname);

  qi::ObjectPtr *obj = new qi::ObjectPtr();
  jobject proxy = 0;

  try
  {
    *obj = s->service(serviceName);
    JNIObject jniProxy(obj);
    return jniProxy.object();
  }
  catch (std::runtime_error &e)
  {
    delete obj;
    throwJavaError(env, e.what());
    return proxy;
  }
}

jboolean  Java_com_aldebaran_qimessaging_Session_registerService(JNIEnv *env, jobject QI_UNUSED(jobj), jlong pSession, jstring jname, jobject object)
{
  qi::Session*    session = reinterpret_cast<qi::Session*>(pSession);
  std::string     name    = toStdString(env, jname);
  JNIObject obj(object);
  jlong ret = 0;

  try
  {
    ret = session->registerService(name, obj.objectPtr());
  }
  catch (std::runtime_error &e)
  {
    throwJavaError(env, e.what());
    return false;
  }

  if (ret <= 0)
  {
    throwJavaError(env, "Cannot register service");
    return false;
  }

  return true;
}
