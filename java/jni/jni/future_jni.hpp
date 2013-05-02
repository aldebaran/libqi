/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#ifndef _FUTURE_JNI_HPP_
#define _FUTURE_JNI_HPP_

#include <jni.h>
#include <qi/future.hpp>
#include <qimessaging/api.hpp>

//jobject   newJavaFuture(qi::Future<qi::GenericValuePtr> *fut);

extern "C"
{
  QIMESSAGING_API jboolean Java_com_aldebaran_qimessaging_Future_qiFutureCallCancel(JNIEnv *env, jobject obj, jlong pFuture, jboolean mayInterup);
  QIMESSAGING_API jobject  Java_com_aldebaran_qimessaging_Future_qiFutureCallGet(JNIEnv *env, jobject obj, jlong pFuture);
  QIMESSAGING_API jobject  Java_com_aldebaran_qimessaging_Future_qiFutureCallGetWithTimeout(JNIEnv *env, jobject obj, jlong pFuture, jint timeout);
  QIMESSAGING_API jboolean Java_com_aldebaran_qimessaging_Future_qiFutureCallIsCancelled(JNIEnv *env, jobject obj, jlong pFuture);
  QIMESSAGING_API jboolean Java_com_aldebaran_qimessaging_Future_qiFutureCallIsDone(JNIEnv *env, jobject obj, jlong pFuture);
  QIMESSAGING_API jboolean Java_com_aldebaran_qimessaging_Future_qiFutureCallConnect(JNIEnv *env, jobject obj, jlong pFuture, jobject callable, jstring className, jobjectArray args);
} // !extern "C"

#endif //!_FUTURE_JNI_HPP_
