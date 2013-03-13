/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qitype/signature.hpp>
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

void getJavaSignature(std::string &sig, const std::string& sigInfo)
{
  unsigned int i = 0;

  while (i < sigInfo.size())
  {
    switch (sigInfo[i])
    {
    case qi::Signature::Type_Bool:
      sig.append("Ljava/lang/Boolean;");
      break;
    case qi::Signature::Type_Int8:
      sig.append("Ljava/lang/Character;");
      break;
    case qi::Signature::Type_Float:
      sig.append("Ljava/lang/Float;");
      break;
    case qi::Signature::Type_Double:
      sig.append("Ljava/lang/Double;");
      break;
    case qi::Signature::Type_Int32:
      sig.append("Ljava/lang/Integer;");
      break;
    case qi::Signature::Type_String:
      sig.append("Ljava/lang/String;");
      break;
    case qi::Signature::Type_Void:
      sig.append("V");
      break;
    case qi::Signature::Type_Dynamic:
      sig.append("L");
      break;
    case qi::Signature::Type_Map:
    {
      sig.append("Ljava/util/Map;");
      while (i < sigInfo.size() && sigInfo[i] != qi::Signature::Type_Map_End)
        i++;
      break;
    }
    case qi::Signature::Type_Tuple:
    {
      sig.append("Ljava/lang/Object;");
      while (i < sigInfo.size() && sigInfo[i] != qi::Signature::Type_Tuple_End)
        i++;
      break;
    }
    case qi::Signature::Type_List:
    {
      sig.append("Ljava/util/ArrayList;");
      while (i < sigInfo.size() && sigInfo[i] != qi::Signature::Type_List_End)
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
  std::string              sig;

  // Compute every arguments
  sig.append("(");
  getJavaSignature(sig, sigInfo[2].substr(1, sigInfo[2].size()-2));
  sig.append(")");

  // Finaly add return signature (or 'V' if empty)
  if (sigInfo[0] == "")
    sig.append("V");
  else
    getJavaSignature(sig, sigInfo[0]);

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

  JVM()->GetEnv((void **) &env, QI_JNI_MIN_VERSION);

  jclass cls = env->FindClass(denomination.c_str());
  jmethodID mid = env->GetMethodID(cls, "init","()V");
  return env->NewObject(cls, mid);
}
