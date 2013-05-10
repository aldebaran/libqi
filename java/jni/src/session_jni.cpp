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
#include "jnitools.hpp"
#include "session_jni.hpp"

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

jlong Java_com_aldebaran_qimessaging_Session_qiSessionService(JNIEnv *env, jobject jobj, jlong pSession, jstring jurl)
{
  if (pSession == 0)
  {
    throwJavaError(env, "Given qi::Session doesn't exists (pointer null).");
    return 0;
  }

  qi::Session *s = reinterpret_cast<qi::Session*>(pSession);
  std::string serviceName = toStdString(env, jurl);

  qi::ObjectPtr *obj = new qi::ObjectPtr();
  qi::ObjectPtr &o = *(reinterpret_cast<qi::ObjectPtr *>(obj));

  try
  {
    o = s->service(serviceName);
    if (!o)
    {
      delete obj;
      throwJavaError(env, "Cannot get service");
      return 0;
    }
    return (jlong) obj;
  }
  catch (std::runtime_error &e)
  {
    throwJavaError(env, e.what());
    return 0;
  }
}

jlong Java_com_aldebaran_qimessaging_Session_qiSessionRegisterService
(JNIEnv *env, jobject obj, jlong pSession, jlong pObject, jstring jname)
{
  qi::Session*    session = reinterpret_cast<qi::Session*>(pSession);
  std::string     name    = toStdString(env, jname);
  qi::ObjectPtr*  object  = reinterpret_cast<qi::ObjectPtr*>(pObject);
  jlong ret = 0;

  char *cname = qi::os::strdup(name.c_str());
  try
  {
    ret = session->registerService(name, *object);
  }
  catch (std::runtime_error &e)
  {
    throwJavaError(env, e.what());
  }

  free(cname);
  return ret;
}
