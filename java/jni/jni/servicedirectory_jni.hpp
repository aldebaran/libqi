/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_SERVICEDIRECTORY_HPP_
#define _JAVA_JNI_SERVICEDIRECTORY_HPP_

#include <qimessaging/api.hpp>
#include <jni.h>

extern "C"
{
  QIMESSAGING_API jlong   Java_com_aldebaran_qimessaging_ServiceDirectory_qiTestSDCreate(JNIEnv *env, jobject obj);
  QIMESSAGING_API void    Java_com_aldebaran_qimessaging_ServiceDirectory_qiTestSDDestroy(jlong pServiceDirectory);
  QIMESSAGING_API jstring Java_com_aldebaran_qimessaging_ServiceDirectory_qiListenUrl(JNIEnv *env, jobject obj, jlong pServiceDirectory);
  QIMESSAGING_API void    Java_com_aldebaran_qimessaging_ServiceDirectory_qiTestSDClose(jlong pServiceDirectory);

} // !extern "C"

#endif // !_JAVA_JNI_SERVICEDIRECTORY_HPP_
