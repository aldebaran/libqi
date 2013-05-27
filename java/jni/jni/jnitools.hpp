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

// Define a portable JNIEnv* pointer
#ifdef ANDROID
  typedef JNIEnv** envPtr;
#else
  typedef void** envPtr;
#endif

// Define JNI minimum version used by qimessaging bindings
#define QI_JNI_MIN_VERSION JNI_VERSION_1_6
// QI_OBJECT_CLASS defines complete name of java generic object class
#define QI_OBJECT_CLASS "com/aldebaran/qimessaging/Object"

// String conversion
std::string   toStdString(JNIEnv *env, jstring inputString);
jstring       toJavaString(JNIEnv *env, const std::string &inputString);

// Global JVM Pointer
JavaVM*       JVM(JNIEnv* env = 0);

// Signature conversion
std::string   toJavaSignature(const std::string &signature);
std::string   propertyBaseSignature(JNIEnv *env, jclass propertyBase);

// Java exception thrower
jint          throwJavaError(JNIEnv *env, const char *message);

#endif // !_JAVA_JNI_JNITOOLS_HPP_
