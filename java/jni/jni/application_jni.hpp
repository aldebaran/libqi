/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_APPLICATION_HPP_
#define _JAVA_JNI_APPLICATION_HPP_

#include <qimessaging/api.hpp>
#include <jni.h>

extern "C"
{
  QIMESSAGING_API jlong Java_com_aldebaran_qimessaging_Application_qiApplicationCreate(JNIEnv *env, jobject jobj);
  QIMESSAGING_API void  Java_com_aldebaran_qimessaging_Application_qiApplicationDestroy(jlong pApplication);
  QIMESSAGING_API void  Java_com_aldebaran_qimessaging_Application_qiApplicationRun(jlong pApplication);
  QIMESSAGING_API void  Java_com_aldebaran_qimessaging_Application_qiApplicationStop(jlong pApplication);

} // !extern "C"

#endif // !_JAVA_JNI_APPLICATION_HPP_
