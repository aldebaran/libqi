/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#include <stdexcept>
#include <sstream>
#include <qi/log.hpp>
#include <jnitools.hpp>
#include <tuple_jni.hpp>

JNITuple::JNITuple(jobject obj)
{
  JVM()->GetEnv((void**) &_env, QI_JNI_MIN_VERSION);
  _obj = obj;
  _cls = _env->FindClass("com/aldebaran/qimessaging/Tuple");
}

JNITuple::JNITuple(int size)
{
  std::stringstream name;
  jmethodID mid = 0;

  name << "com/aldebaran/qimessaging/Tuple" << size;

  JVM()->GetEnv((void**) &_env, QI_JNI_MIN_VERSION);
  _cls = _env->FindClass(name.str().c_str());
  mid = _env->GetMethodID(_cls, "<init>", "()V");

  if (!mid)
  {
    qiLogError("qimessaging.jni") << "JNITuple : Cannot find " << name.str() << " constructor";
    throwJavaError(_env, "JNITuple : Cannot find Tuple constructor");
  }

  _obj = _env->NewObject(_cls, mid);
}

JNITuple::~JNITuple()
{
  _env->DeleteLocalRef(_cls);
}

int JNITuple::size()
{
  jmethodID mid = _env->GetMethodID(_cls, "size", "()I");

  if (!mid)
  {
    qiLogFatal("qimessaging.jni") << "JNITuple : Cannot call method size()I";
    throw std::runtime_error("JNITuple : Cannot call method size()I");
  }

  return _env->CallIntMethod(_obj, mid);
}

jobject JNITuple::get(int index)
{
  jmethodID mid = _env->GetMethodID(_cls, "get", "(I)Ljava/lang/Object;");

  if (!mid)
  {
    qiLogFatal("qimessaging.jni") << "JNITuple : Cannot call method get(I)Ljava/lang/Object;";
    throw std::runtime_error("JNITuple : Cannot call method get(I)Ljava/lang/Object;");
  }

  return _env->CallObjectMethod(_obj, mid, index);
}

void JNITuple::set(int index, jobject obj)
{
  jmethodID mid = _env->GetMethodID(_cls, "set", "(ILjava/lang/Object;)V");

  if (!mid)
  {
    qiLogFatal("qimessaging.jni") << "JNITuple : Cannot call method set(ILjava/lang/Object;)V";
    throw std::runtime_error("JNITuple : Cannot call method set(ILjava/lang/Object;)V");
  }

  _env->CallVoidMethod(_obj, mid, index, obj);
}

jobject JNITuple::object()
{
  return _obj;
}
