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
#include <map_jni.hpp>

qiLogCategory("qimessaging.jni");

JNIHashTable::JNIHashTable()
{
   JVM()->GetEnv((void**) &_env, QI_JNI_MIN_VERSION);
  _cls = _env->FindClass("java/util/Hashtable");

  jmethodID mid = _env->GetMethodID(_cls, "<init>", "()V");
  if (!mid)
  {
    qiLogFatal() << "JNIHashTable::JNIHashTable : Cannot call constructor";
    throw std::runtime_error("JNIHashTable::JNIHashTable : Cannot call constructor");
  }

  _obj = _env->NewObject(_cls, mid);
}

JNIHashTable::JNIHashTable(jobject obj)
{
   JVM()->GetEnv((void**) &_env, QI_JNI_MIN_VERSION);
   _obj = obj;
  _cls = _env->FindClass("java/util/Hashtable");
}

JNIHashTable::~JNIHashTable()
{
  _env->DeleteLocalRef(_cls);
}

bool JNIHashTable::setItem(jobject key, jobject value)
{
  jmethodID mid = _env->GetMethodID(_cls, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

  if (!key || !value)
  {
    qiLogFatal() << "JNIHashTable::setItem() : Given key/value pair is null";
    return false;
  }

  if (!mid)
  {
    qiLogFatal() << "JNIHashTable::setItem() : Cannot call put";
    throw std::runtime_error("JNIHashTable::setItem() : Cannot call put");
  }

  _env->CallVoidMethod(_obj, mid, key, value);
  return true;
}

jobject JNIHashTable::object()
{
  return _obj;
}

int     JNIHashTable::size()
{
  jmethodID mid = _env->GetMethodID(_cls, "size", "()I");

  if (!mid) // or throw std::runtime_error ?
    return (-1);

  return _env->CallIntMethod(_obj, mid);
}

JNIEnumeration JNIHashTable::keys()
{
  jmethodID mid = _env->GetMethodID(_cls, "keys", "()Ljava/util/Enumeration;");

  if (!mid)
  {
    qiLogFatal() << "JNIHashTable : Cannot call method keys";
    throw std::runtime_error("JNIHashTable : Cannot call method keys");
  }

  return JNIEnumeration(_env->CallObjectMethod(_obj, mid));
}

jobject JNIHashTable::at(jobject key)
{
  jmethodID mid = _env->GetMethodID(_cls, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

  if (!key)
  {
    qiLogFatal() << "JNIHashTable::at() : Given key is null";
    return 0;
  }

  if (!mid)
  {
    qiLogFatal() << "JNIHashTable::at() : Cannot call method get";
    throw std::runtime_error("JNIHashTable::at() : Cannot call method get");
  }

  return _env->CallObjectMethod(_obj, mid, key);
}

jobject JNIHashTable::at(int pos)
{
  throw std::runtime_error("JNIHashTable::at() : Not implemented");
}

bool    JNIHashTable::next(int* pos, jobject* key, jobject* value)
{
  throw std::runtime_error("JNIHashTable::next() : Not implemented");
}
