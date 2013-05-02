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

#include <qimessaging/c/error_c.h>
#include <qimessaging/c/session_c.h>

#include <jni.h>
#include "jnitools.hpp"
#include "session_jni.hpp"

jlong Java_com_aldebaran_qimessaging_Session_qiSessionCreate()
{
  return (jlong) qi_session_create();
}

void Java_com_aldebaran_qimessaging_Session_qiSessionDestroy(jlong pSession)
{
  qi_session_destroy(reinterpret_cast<qi_session_t *>(pSession));
}

jboolean Java_com_aldebaran_qimessaging_Session_qiSessionIsConnected(JNIEnv* QI_UNUSED(env), jobject QI_UNUSED(obj), jlong pSession)
{
  return (jboolean) qi_session_is_connected(reinterpret_cast<qi_session_t *>(pSession));
}

jboolean Java_com_aldebaran_qimessaging_Session_qiSessionConnect(JNIEnv *env, jobject QI_UNUSED(obj), jlong pSession, jstring jurl)
{
  if (pSession == 0)
  {
    throwJavaError(env, "Given qi::Session doesn't exists (pointer null).");
    return false;
  }

  bool ret = qi_session_connect(reinterpret_cast<qi_session_t *>(pSession), toStdString(env, jurl).c_str());

  if ((bool) ret == false)
  {
    const char* error_msg = qi_c_error() != 0 ? qi_c_error() : "Connection unexpectedly failed.";
    throwJavaError(env, error_msg);
    return false;
  }

  return true;
}

void Java_com_aldebaran_qimessaging_Session_qiSessionClose(JNIEnv* QI_UNUSED(env), jobject QI_UNUSED(obj), jlong pSession)
{
  qi_session_close(reinterpret_cast<qi_session_t *>(pSession));
}

jlong Java_com_aldebaran_qimessaging_Session_qiSessionService(JNIEnv *env, jobject obj, jlong pSession, jstring jurl)
{
  if (pSession == 0)
  {
    throwJavaError(env, "Given qi::Session doesn't exists (pointer null).");
    return false;
  }

  std::string serviceName = toStdString(env, jurl);
  return (jlong) qi_session_get_service(reinterpret_cast<qi_session_t *>(pSession), serviceName.c_str());;
}

jlong Java_com_aldebaran_qimessaging_Session_qiSessionRegisterService
(JNIEnv *env, jobject obj, jlong pSession, jlong pObject, jstring jname)
{
  qi_session_t* session = reinterpret_cast<qi_session_t*>(pSession);
  std::string   name    = toStdString(env, jname);
  qi_object_t*  object  = reinterpret_cast<qi_object_t *>(pObject);

  char *cname = qi::os::strdup(name.c_str());
  jlong ret = qi_session_register_service(session, name.c_str(), object);
  free(cname);
  return ret;
}
