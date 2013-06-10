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

qiLogCategory("qimessaging.jni");

std::map<std::string, jobject> supportedTypes;

jint JNI_OnLoad(JavaVM* QI_UNUSED(vm), void* QI_UNUSED(reserved))
{
  qiLogInfo() << "qimessagingjni loaded.";
  return QI_JNI_MIN_VERSION;
}

/*
 * To work correctly, QiMessaging<->java type system needs to compare type class template.
 * Unfortunately, call template cannot be retrieve on native android thread, that is why
 * type instance are stored in supportedTypes map.
 */
void Java_com_aldebaran_qimessaging_EmbeddedTools_initTypeSystem(JNIEnv* env, jobject QI_UNUSED(jobj), jobject str, jobject i, jobject f, jobject d, jobject l, jobject m, jobject al, jobject tuple, jobject obj, jobject b)
{
  JVM(env);

  supportedTypes["String"] = env->NewGlobalRef(str);
  supportedTypes["Integer"] = env->NewGlobalRef(i);
  supportedTypes["Float"] = env->NewGlobalRef(f);
  supportedTypes["Double"] = env->NewGlobalRef(d);
  supportedTypes["Long"] = env->NewGlobalRef(l);
  supportedTypes["Map"] = env->NewGlobalRef(m);
  supportedTypes["List"] = env->NewGlobalRef(al);
  supportedTypes["Tuple"] = env->NewGlobalRef(tuple);
  supportedTypes["Object"] = env->NewGlobalRef(obj);
  supportedTypes["Boolean"] = env->NewGlobalRef(b);

  for (std::map<std::string, jobject>::iterator it = supportedTypes.begin(); it != supportedTypes.end(); ++it)
  {
    if (it->second == 0)
      qiLogFatal() << it->first << ": Initialization failed.";
  }
}

void Java_com_aldebaran_qimessaging_EmbeddedTools_initTupleInTypeSystem(JNIEnv* env, jobject QI_UNUSED(jobj), jobject t1, jobject t2, jobject t3, jobject t4, jobject t5, jobject t6, jobject t7, jobject t8)
{
  JVM(env);

  supportedTypes["Tuple1"] = env->NewGlobalRef(t1);
  supportedTypes["Tuple2"] = env->NewGlobalRef(t2);
  supportedTypes["Tuple3"] = env->NewGlobalRef(t3);
  supportedTypes["Tuple4"] = env->NewGlobalRef(t4);
  supportedTypes["Tuple5"] = env->NewGlobalRef(t5);
  supportedTypes["Tuple6"] = env->NewGlobalRef(t6);
  supportedTypes["Tuple7"] = env->NewGlobalRef(t7);
  supportedTypes["Tuple8"] = env->NewGlobalRef(t8);

  for (std::map<std::string, jobject>::iterator it = supportedTypes.begin(); it != supportedTypes.end(); ++it)
  {
    if (it->second == 0)
      qiLogError() << it->first << ": Initialization failed.";
  }
}

/*
 * JNIEnv structure is thread dependent.
 * To use a JNIEnv* pointer in another thread (such as QiMessaging callback)
 * it must be attach to current thread (via JavaVM->AttachCurrentThread())
 * Therefore we keep a pointer to the JavaVM, wich is thread safe.
 */
JavaVM*       JVM(JNIEnv* env)
{
  static JavaVM* gJVM = 0;

  if (env != 0 && gJVM == 0)
    env->GetJavaVM(&gJVM);

  if (gJVM == 0 && env == 0)
    qiLogError() << "JVM singleton wasn't initialised";
  return gJVM;
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
      sig.append("Lcom/aldebaran/qimessaging/Tuple;");
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
    case qi::Signature::Type_Object:
    {
      sig.append("L");
      sig.append(QI_OBJECT_CLASS);
      sig.append(";");
      break;
    }
    default:
      qiLogFatal() << "Unknown conversion for [" << sigInfo[i] << "]";
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
    qiLogFatal() << "Cannot throw any exceptions";
    return 1;
  }

  return env->ThrowNew(exClass, message);
}

std::string propertyBaseSignature(JNIEnv* env, jclass propertyBase)
{
  std::string sig;

  jclass stringClass = qi::jni::clazz("String");
  jclass int32Class = qi::jni::clazz("Integer");
  jclass floatClass = qi::jni::clazz("Float");
  jclass doubleClass = qi::jni::clazz("Double");
  jclass boolClass = qi::jni::clazz("Boolean");
  jclass longClass = qi::jni::clazz("Long");
  jclass mapClass = qi::jni::clazz("Map");
  jclass listClass = qi::jni::clazz("List");
  jclass tupleClass = qi::jni::clazz("Tuple");
  jclass objectClass = qi::jni::clazz("Object");

  if (env->IsAssignableFrom(propertyBase, stringClass) == true)
    sig = static_cast<char>(qi::Signature::Type_String);
  if (env->IsAssignableFrom(propertyBase, int32Class) == true)
    sig = static_cast<char>(qi::Signature::Type_Int32);
  if (env->IsAssignableFrom(propertyBase, floatClass) == true)
    sig = static_cast<char>(qi::Signature::Type_Float);
  if (env->IsAssignableFrom(propertyBase, boolClass) == true)
    sig = static_cast<char>(qi::Signature::Type_Bool);
  if (env->IsAssignableFrom(propertyBase, longClass) == true)
    sig = static_cast<char>(qi::Signature::Type_Int64);
  if (env->IsAssignableFrom(propertyBase, objectClass) == true)
    sig = static_cast<char>(qi::Signature::Type_Object);
  if (env->IsAssignableFrom(propertyBase, doubleClass) == true)
    sig = static_cast<char>(qi::Signature::Type_Float);
  if (env->IsAssignableFrom(propertyBase, mapClass) == true)
  {
    sig = static_cast<char>(qi::Signature::Type_Map);
    sig += static_cast<char>(qi::Signature::Type_Dynamic);
    sig += static_cast<char>(qi::Signature::Type_Map_End);
  }
  if (env->IsAssignableFrom(propertyBase, listClass) == true)
  {
    sig = static_cast<char>(qi::Signature::Type_List);
    sig += static_cast<char>(qi::Signature::Type_Dynamic);
    sig += static_cast<char>(qi::Signature::Type_List_End);
  }
  if (env->IsAssignableFrom(propertyBase, tupleClass) == true)
  {
    sig = static_cast<char>(qi::Signature::Type_Tuple);
    sig += static_cast<char>(qi::Signature::Type_Dynamic);
    sig += static_cast<char>(qi::Signature::Type_Tuple_End);
  }

  qi::jni::releaseClazz(stringClass);
  qi::jni::releaseClazz(int32Class);
  qi::jni::releaseClazz(floatClass);
  qi::jni::releaseClazz(doubleClass);
  qi::jni::releaseClazz(boolClass);
  qi::jni::releaseClazz(longClass);
  qi::jni::releaseClazz(mapClass);
  qi::jni::releaseClazz(listClass);
  qi::jni::releaseClazz(tupleClass);
  qi::jni::releaseClazz(objectClass);

  return sig;
}

// JNIEnv.FindClass has not same behavior on desktop and android.
// Here is a little wrapper to find wanted class anywhere.

namespace qi {
  namespace jni {

    // Get JNI environment pointer, valid in current thread.
    JNIEnv*     env()
    {
      JNIEnv* env = 0;

      JVM()->GetEnv(reinterpret_cast<void**>(&env), QI_JNI_MIN_VERSION);
      if (!env)
      {
        qiLogError() << "Cannot get JNI environment from JVM";
        return 0;
      }

      if (JVM()->AttachCurrentThread(reinterpret_cast<envPtr>(&env), (void*)0) != JNI_OK)
      {
        qiLogError() << "Cannot attach current thread to JVM";
        return 0;
      }

      return env;
    }

    // JNIEnv.FindClass has not same behavior on desktop and android.
    // Here is a little wrapper to find wanted class anywhere.
    jclass     clazz(const std::string& className)
    {
      JNIEnv*   env = qi::jni::env();
      jclass   cls = 0;

      if (!env)
        return 0;

      jobject obj = supportedTypes[className];
      if (!obj)
      {
        qiLogError() << className << " unknown in type system";
        return cls;
      }

      return env->GetObjectClass(obj);
    }

    // JNIEnv.FindClass has not same behavior on desktop and android.
    // Here is a little wrapper to find wanted class anywhere.
    jclass      clazz(jobject object)
    {
      JNIEnv*   env = qi::jni::env();

      if (!env)
        return 0;

      return (jclass) env->GetObjectClass(object);
    }

    // Release local ref to avoid JNI internal reference table overflow
    void        releaseClazz(jclass clazz)
    {
      JNIEnv*   env = qi::jni::env();

      if (!env || !clazz)
      {
        qiLogError() << "Cannot release local class reference. (Env: " << env << ", Clazz: " << clazz << ")";
        return;
      }

      env->DeleteLocalRef(clazz);
    }

    // Convert jstring into std::string
    // Use of std::string ensures ref leak safety.
    std::string toString(jstring inputString)
    {
      std::string string;
      const char *cstring = 0;
      JNIEnv*   env = qi::jni::env();

      if (!env)
        return string;

      if (!(cstring = env->GetStringUTFChars(inputString, 0)))
      {
        qiLogError() << "Cannot convert Java string into string.";
        return string;
      }

      string = cstring;
      env->ReleaseStringUTFChars(inputString, cstring);
      return string;
    }

    // Convert std::string into jstring
    // Use qi::jni::releaseString to avoir ref leak.
    jstring     toJstring(const std::string& input)
    {
      jstring   string = 0;
      JNIEnv*   env = qi::jni::env();

      if (!env)
        return string;

      if (!(string = env->NewStringUTF(input.c_str())))
        qiLogError() << "Cannot convert string into Java string.";

      return string;
    }

    // Remove local ref on jstring created with qi::jni::toJstring
    void        releaseString(jstring input)
    {
      JNIEnv*   env = qi::jni::env();

      if (!env)
        return;

      env->DeleteLocalRef(input);
    }

    // Release local ref on JNI object
    void        releaseObject(jobject obj)
    {
      JNIEnv*   env = qi::jni::env();

      if (!env)
        return;

      env->DeleteLocalRef(obj);
    }

    // Return true is jobject is a QiMessaging tuple.
    bool        isTuple(jobject object)
    {
      JNIEnv*     env = qi::jni::env();
      jclass      cls = 0;
      std::string className;

      if (!env)
        return false;

      for (std::map<std::string, jobject>::iterator it = supportedTypes.begin(); it != supportedTypes.end(); ++it)
      {
        className = it->first;

        // searching for tuple
        if (className.find("Tuple") == std::string::npos)
          continue;

        cls = env->GetObjectClass(it->second);
        if (env->IsInstanceOf(object, cls))
        {
          qi::jni::releaseClazz(cls);
          return true;
        }

        qi::jni::releaseClazz(cls);
      }

      return false;
    }

  }// !jni
}// !qi
