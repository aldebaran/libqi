/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_JNITOOLS_HPP_
#define _JAVA_JNI_JNITOOLS_HPP_

#include <iostream>
#include <qimessaging/api.hpp>
#include <jni.h>

#ifdef ANDROID
# include <android/log.h>
#endif

// Define a portable JNIEnv* pointer (API between arm and intel differs)
#ifdef ANDROID
  typedef JNIEnv** envPtr;
#else
  typedef void** envPtr;
#endif

// Define JNI minimum version used by qimessaging bindings
#define QI_JNI_MIN_VERSION JNI_VERSION_1_6
// QI_OBJECT_CLASS defines complete name of java generic object class
#define QI_OBJECT_CLASS "com/aldebaran/qimessaging/Object"

// JNI utils
extern "C"
{
  QIMESSAGING_API jint JNI_OnLoad(JavaVM *vm, void*);
  QIMESSAGING_API void Java_com_aldebaran_qimessaging_EmbeddedTools_initTypeSystem(JNIEnv* env, jobject jobj, jobject str, jobject i, jobject f, jobject d,
                                                                                   jobject l, jobject m, jobject al, jobject tuple, jobject obj, jobject b);
  QIMESSAGING_API void Java_com_aldebaran_qimessaging_EmbeddedTools_initTupleInTypeSystem(JNIEnv* env, jobject jobj, jobject t1, jobject t2, jobject t3, jobject t4,
                                                                                   jobject t5, jobject t6, jobject t7, jobject t8);
} // !extern C


namespace qi {
  namespace jni {

    // String conversion
    std::string toString(jstring input);
    jstring     toJstring(const std::string& input);
    void        releaseString(jstring input);
    // TypeSystem tools
    jclass      clazz(const std::string &name);
    jclass      clazz(jobject object);
    void        releaseClazz(jclass clazz);
    // JVM Environment management
    JNIEnv*     env();
    void        releaseObject(jobject obj);
    // Signature
    std::string javaSignature(const std::string& qiSignature);
    std::string qiSignature(jclass clazz);

  }// !jni
}// !qi


JavaVM*       JVM(JNIEnv* env = 0);

// Signature conversion
std::string   toJavaSignature(const std::string &signature);
std::string   propertyBaseSignature(JNIEnv *env, jclass propertyBase);

// Java exception thrower
jint          throwJavaError(JNIEnv *env, const char *message);

extern std::map<std::string, jobject> supportedTypes;
#endif // !_JAVA_JNI_JNITOOLS_HPP_
