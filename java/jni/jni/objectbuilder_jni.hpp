/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_GENERICOBJECT_HPP_
#define _JAVA_JNI_GENERICOBJECT_HPP_

#include <qimessaging/api.hpp>
#include <jni.h>

extern "C"
{

  QIMESSAGING_API jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderCreate();
  QIMESSAGING_API jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderGetObject(JNIEnv *env, jobject jobj, jlong pObjectBuilder);
  QIMESSAGING_API void    Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderDestroy(jlong pObjectBuilder);

} // !extern "C"

#endif // !_JAVA_JNI_GENERICOBJECT_HPP_

