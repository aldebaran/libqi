/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_JNITOOLS_HPP_
#define _JAVA_JNI_JNITOOLS_HPP_

#include <iostream>
#include <qimessaging/api.hpp>
#include <qimessaging/c/message_c.h>
#include <jni.h>

#ifdef ANDROID
# include <android/log.h>
#endif

// Define a portable JNIEnv* pointer
#ifdef ANDROID
  typedef JNIEnv** envPtr;
#else
  typedef void** envPtr;
#endif

// Define JNI minimum version used by qimessaging bindings
#define QI_JNI_MIN_VERSION JNI_VERSION_1_6

std::string   toStdString(JNIEnv *env, jstring inputString);
jstring       toJavaString(JNIEnv *env, const std::string &inputString);
qi_message_t* toQiMessage(JNIEnv *env, jobject obj, qi_message_t *m = 0);
jobject       toJavaObject(JNIEnv *env, const std::string &sigreturn, qi_message_t *message);
JavaVM*       JVM(JNIEnv* env = 0);
std::string   toJavaSignature(const std::string &signature);
jint          throwJavaError(JNIEnv *env, const char *message);
jobject       loadJavaObject(const std::string& denomination);
void          getJavaSignature(std::string &sig, const std::string &sigInfo);

extern "C"
{
  QIMESSAGING_API jlong Java_com_aldebaran_qimessaging_Session_PocAndroidConnection(JNIEnv *env, jobject obj, jlong pSession, jstring url);
} // !extern "C"

#endif // !_JAVA_JNI_JNITOOLS_HPP_
