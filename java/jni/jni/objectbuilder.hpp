/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_OBJECTBUILDER_HPP_
#define _JAVA_JNI_OBJECTBUILDER_HPP_

#include <qimessaging/api.hpp>
#include <jni.h>

extern "C"
{

  QIMESSAGING_API jlong   Java_com_aldebaran_qimessaging_GenericObjectBuilder_create();
  QIMESSAGING_API jobject Java_com_aldebaran_qimessaging_GenericObjectBuilder_object(JNIEnv *env, jobject jobj, jlong pObjectBuilder);
  QIMESSAGING_API void    Java_com_aldebaran_qimessaging_GenericObjectBuilder_destroy(JNIEnv *env, jobject jobj, jlong pObjectBuilder);
  QIMESSAGING_API jlong   Java_com_aldebaran_qimessaging_GenericObjectBuilder_advertiseMethod(JNIEnv *env, jobject obj, jlong pObjectBuilder, jstring method, jobject instance, jstring service, jstring desc);
  QIMESSAGING_API jlong   Java_com_aldebaran_qimessaging_GenericObjectBuilder_advertiseSignal(JNIEnv *env, jobject obj, jlong pObjectBuilder, jstring eventSignature);
  QIMESSAGING_API jlong   Java_com_aldebaran_qimessaging_GenericObjectBuilder_advertiseProperty(JNIEnv *env, jobject obj, jlong pObjectBuilder, jstring name, jclass propertyBase);

} // !extern "C"

#endif // !_JAVA_JNI_OBJECTBUILDER_HPP_

