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
#include <list_jni.hpp>

JNIList::JNIList()
{
  JVM()->GetEnv((void**) &_env, QI_JNI_MIN_VERSION);
 _cls = _env->FindClass("java/util/ArrayList");

 jmethodID mid = _env->GetMethodID(_cls, "<init>", "()V");
 if (!mid)
 {
   qiLogFatal("qimessaging.jni") << "JNIList::JNIList: Cannot call constructor";
   throw std::runtime_error("JNIList::JNIList: Cannot call constructor");
 }

 _obj = _env->NewObject(_cls, mid);
}

JNIList::JNIList(jobject obj)
{
  JVM()->GetEnv((void**) &_env, QI_JNI_MIN_VERSION);
 _obj = obj;
 _cls = _env->FindClass("java/util/ArrayList");
}

JNIList::~JNIList()
{
  _env->DeleteLocalRef(_cls);
}

int JNIList::size()
{
  jmethodID mid = _env->GetMethodID(_cls, "size", "()I");

  if (!mid)
  {
    qiLogFatal("qimessaging.jni") << "JNIList::size() : Cannot call size()";
    throw std::runtime_error("JNIList::size() : Cannot call size()");
  }

  return _env->CallIntMethod(_obj, mid);
}

jobject JNIList::get(int index)
{
  jmethodID mid = _env->GetMethodID(_cls, "get", "(I)Ljava/lang/Object;");

  if (!mid)
  {
    qiLogFatal("qimessaging.jni") << "JNIList::get() : Cannot call get()";
    throw std::runtime_error("JNIList::get() : Cannot call get()");
  }

  return _env->CallObjectMethod(_obj, mid, index);
}

jobject JNIList::object()
{
  return _obj;
}

bool JNIList::push_back(jobject current)
{
  jmethodID mid = _env->GetMethodID(_cls, "add", "(Ljava/lang/Object;)Z");

  if (!mid)
  {
    qiLogFatal("qimessaging.jni") << "JNIList::push_back() : Cannot call add()";
    throw std::runtime_error("JNIList::push_back() : Cannot call add()");
  }

  return _env->CallBooleanMethod(_obj, mid, current);
}
