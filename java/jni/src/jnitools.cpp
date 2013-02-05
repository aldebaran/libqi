/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qitype/signature.hpp>
#include <qimessaging/c/signature_c.h>
#include <qimessaging/session.hpp>
#include "jnitools.hpp"

/*
 * JNIEnv structure is thread dependent.
 * To use a JNIEnv* pointer in another thread (such as QiMessaging callback)
 * it must be attach to current thread (via JavaVM->AttachCurrentThread())
 * Therefore we keep a pointer to the JavaVM, witch is thread safe.
 */
JavaVM*       JVM(JNIEnv* env)
{
  static JavaVM* gJVM = 0;

  if (env != 0 && gJVM == 0)
    env->GetJavaVM(&gJVM);

  if (gJVM == 0 && env == 0)
    qiLogError("qimessaging.jni.JVM") << "JVM singleton wasn't initialised";
  return gJVM;
}

std::string toStdString(JNIEnv *env, jstring inputString)
{
  std::string string;
  const char *cstring = 0;

  if (!env)
  {
    qiLogError("qimessaging.jni") << "Java environment missing.";
    return string;
  }

  if (!(cstring = env->GetStringUTFChars(inputString, 0)))
  {
    qiLogError("qimessaging.jni") << "Cannot convert Java string into string.";
  }

  string = cstring;
  return string;
}

jstring     toJavaString(JNIEnv *env, const std::string &inputString)
{
  jstring   string = 0;

  if (!env)
  {
    qiLogError("qimessaging.jni") << "Java environment missing.";
    return string;
  }

  if (!(string = env->NewStringUTF(inputString.c_str())))
    qiLogError("qimessaging.jni") << "Cannot convert string into Java string.";

  return string;
}

qi_message_t* toQiMessage(JNIEnv *env, jobject obj, qi_message_t *m)
{
  // If qi_message_t is not given, create a new one.
  qi_message_t* message = m != 0 ? m : qi_message_create();

  //jclass clazz = env->GetObjectClass(obj);
  jclass int32Class = env->FindClass("java/lang/Integer");
  jclass stringClass = env->FindClass("java/lang/String");

  // Return empty message if no parameter given.
  if (!obj)
    return message;

  if (!env || !int32Class || !stringClass)
  {
    qiLogFatal("qimessaging.java") << "Java environment is down, you're gonna have a bad time.";
    return 0;
  }

  if (env->IsInstanceOf(obj, int32Class) == true)
  {
    int b = 0; // Fixme
    qi_message_write_int32(message, b);
  }

  if (env->IsInstanceOf(obj, stringClass) == true)
  {
    qi_message_write_string(message, toStdString(env, (jstring) obj).c_str());
  }

  return message;
}

jobject toJavaObject(JNIEnv *env, const std::string& sigreturn, qi_message_t *message)
{
  unsigned int i = 0;
  while (i < sigreturn.size())
  {
    switch(sigreturn[i])
    {
    case QI_TUPLE:
      i++;
      continue;
    case QI_BOOL:
      return 0;
    case QI_CHAR:
      return 0;
    case QI_FLOAT:
      return 0;
    case QI_INT:
      return 0;
    case QI_DOUBLE:
      return 0;
    case QI_STRING:
      return toJavaString(env, std::string(qi_message_read_string(message)));
    }
    i++;
  }
  return 0;
}

void getJavaSignature(std::string &sig, char c)
{
  switch (c)
  {
    switch (sigInfo[i])
    {
    case QI_BOOL:
      sig.append("Ljava/lang/Boolean;");
      break;
    case QI_CHAR:
      sig.append("Ljava/lang/Character;");
      break;
    case QI_FLOAT:
      sig.append("Ljava/lang/Float;");
      break;
    case QI_DOUBLE:
      sig.append("Ljava/lang/Double;");
      break;
    case QI_INT:
      sig.append("Ljava/lang/Integer;");
      break;
    case QI_STRING:
      sig.append("Ljava/lang/String;");
      break;
    case QI_VOID:
      sig.append("V");
      break;
    case QI_MESSAGE:
      sig.append("L");
      break;
    case QI_MAP:
    {
      sig.append("Ljava/util/Map;");
      while (i < sigInfo.size() && sigInfo[i] != QI_MAP_END)
        i++;
      break;
    }
    case QI_TUPLE:
    {
      sig.append("Ljava/lang/Object;");
      while (i < sigInfo.size() && sigInfo[i] != QI_TUPLE_END)
        i++;
      break;
    }
    case QI_LIST:
    {
      sig.append("Ljava/util/ArrayList;");
      while (i < sigInfo.size() && sigInfo[i] != QI_LIST_END)
        i++;
      break;
    }
    default:
      qiLogFatal("qimessaging.java") << "Unknown conversion for [" << sigInfo[i] << "]";
      exit(1);
      break;
    }

    i++;
  }
}

std::string   toJavaSignature(const std::string &signature)
{
  std::vector<std::string> sigInfo = qi::signatureSplit(signature);
  unsigned int             i;
  std::string              sig;

  // Compute every arguments
  sig.append("(");
  i = 1;
  while (i < sigInfo[2].size() - 1)
  {
    getJavaSignature(sig, sigInfo[2][i]);
    i++;
  }
  sig.append(")");

  // Finaly add return signature (or 'V' if empty)
  if (sigInfo[0] == "")
    sig.append("V");
  else
    getJavaSignature(sig, sigInfo[0][0]);

  //qiLogDebug("") << "Java signature : " << sig;
  return sig;
}

jint throwJavaError(JNIEnv *env, const char *message)
{
  jclass		 exClass;
  const char*    className = "java/lang/Exception" ;

  exClass = env->FindClass(className);
  if (exClass == NULL)
  {
    qiLogFatal("qimessaging.jni.throw") << "Cannot throw any exceptions";
    return 1;
  }

  return env->ThrowNew(exClass, message);
}

jobject       loadJavaObject(const std::string& denomination)
{
  JNIEnv* env;

  JVM()->GetEnv((void **) &env, JNI_VERSION_1_6);

  jclass cls = env->FindClass(denomination.c_str());
  jmethodID mid = env->GetMethodID(cls, "init","()V");
  return env->NewObject(cls, mid);
}

jlong Java_com_aldebaran_qimessaging_Session_PocAndroidConnection(JNIEnv *env, jobject QI_UNUSED(obj), jlong pSession, jstring jurl)
{
  qi::Session* session = reinterpret_cast<qi::Session*>(pSession);
  std::string  url = toStdString(env, jurl);

  //qiLogInfo("qimessaging.test") << "Testing Android connection...";
  if (session->connect(url) == false)
  {
    throwJavaError(env, "Cannot connect to ServiceDirectory");
    return (jlong) false;
  }

  //qiLogInfo("qimessaging.test") << "Checking connection to service directory";
  if (session->isConnected() == false)
  {
    throwJavaError(env, "Session is not connected");
    return (jlong) false;
  }

  //qiLogInfo("qimessaging.test") << "Getting proxy on serviceTest";
  qi::ObjectPtr proxy;
  try {
    proxy = session->service("serviceTest");
  }
  catch (std::runtime_error &e)
  {
    throwJavaError(env, e.what());
    return (jlong) false;
  }

  //qiLogInfo("qimessaging.test") << "Check proxy";
  if (proxy.get() == 0)
  {
    throwJavaError(env, "Cannot get proxy on serviceTest");
    return (jlong) false;
  }

  qi::Future<std::string> ret = proxy->call<std::string>("reply", "foo");

  ret.wait();
  if (ret.value().compare("foobim") != 0)
  {
    throwJavaError(env, "answer is not 'foobim'");
    return (jlong) false;
  }

  //qiLogInfo("qimessaging.test") << "Poc Android Connection succeed";
  return (jlong) true;
}
