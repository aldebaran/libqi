/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#include <stdexcept>
#include <qi/log.hpp>
#include <jnitools.hpp>
#include <enumeration_jni.hpp>

qiLogCategory("qimessaging.jni");

JNIEnumeration::JNIEnumeration(jobject obj)
{
  JVM()->GetEnv((void**) &_env, QI_JNI_MIN_VERSION);
  _obj = obj;
  _cls = _env->FindClass("java/util/Enumeration");
}

JNIEnumeration::~JNIEnumeration()
{
  _env->DeleteLocalRef(_cls);
}

bool JNIEnumeration::hasNextElement()
{
  jmethodID mid = _env->GetMethodID(_cls, "hasMoreElements", "()Z");

  if (!mid)
  {
    qiLogFatal() << "JNIEnumeration : Cannot call method hasMoreElements";
    throw std::runtime_error("JNIEnumeration : Cannot call method hasMoreElements");
  }

  return _env->CallBooleanMethod(_obj, mid);
}

jobject JNIEnumeration::nextElement()
{
  jmethodID mid = _env->GetMethodID(_cls, "nextElement", "()Ljava/lang/Object;");

  if (!mid)
  {
    qiLogFatal() << "JNIEnumeration : Cannot call method nextElement";
    throw std::runtime_error("JNIEnumeration : Cannot call method nextElement");
  }

  return _env->CallObjectMethod(_obj, mid);
}
