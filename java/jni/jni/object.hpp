/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_OBJECT_HPP_
#define _JAVA_JNI_OBJECT_HPP_

#include <qimessaging/api.hpp>
#include <qi/future.hpp>
#include <jni.h>

extern "C"
{

  QIMESSAGING_API jlong     Java_com_aldebaran_qimessaging_Object_property(JNIEnv* env, jobject jobj, jlong pObj, jstring name);
  QIMESSAGING_API jlong     Java_com_aldebaran_qimessaging_Object_setProperty(JNIEnv* env, jobject jobj, jlong pObj, jstring name, jobject property);
  QIMESSAGING_API jlong     Java_com_aldebaran_qimessaging_Object_asyncCall(JNIEnv* env, jobject jobj, jlong pObj, jstring methodName, jobjectArray args);
  QIMESSAGING_API jstring   Java_com_aldebaran_qimessaging_Object_printMetaObject(JNIEnv* env, jobject jobj, jlong pObj);
  QIMESSAGING_API void      Java_com_aldebaran_qimessaging_Object_destroy(JNIEnv* env, jobject jobj, jlong pObj);
  QIMESSAGING_API jlong     Java_com_aldebaran_qimessaging_Object_connect(JNIEnv *env, jobject obj, jlong pObject, jstring method, jobject instance, jstring service, jstring event);
  QIMESSAGING_API void      Java_com_aldebaran_qimessaging_Object_post(JNIEnv *env, jobject obj, jlong pObject, jstring eventName, jobjectArray args);

} // ! extern "C"

#endif // !_JAVA_JNI_OBJECT_HPP_
