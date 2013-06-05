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
#include <qitype/dynamicobject.hpp>

#include <jnitools.hpp>
#include <object_jni.hpp>

qiLogCategory("qimessaging.jni");

JNIObject::JNIObject(const qi::ObjectPtr& o)
{
  qi::ObjectPtr* newO = new qi::ObjectPtr();
  *newO = o;

  this->build(newO);
}

JNIObject::JNIObject(qi::ObjectPtr *newO)
{
  this->build(newO);
}

JNIObject::JNIObject(jobject value)
{
  JVM()->GetEnv((void**) &_env, QI_JNI_MIN_VERSION);
  JVM()->AttachCurrentThread((envPtr)&_env, (void *) 0);

  _cls = _env->FindClass(QI_OBJECT_CLASS);
  _obj = value;
}

JNIObject::~JNIObject()
{
  _env->DeleteLocalRef(_cls);
}

jobject JNIObject::object()
{
  return _obj;
}

qi::ObjectPtr      JNIObject::objectPtr()
{
  jfieldID fid = _env->GetFieldID(_cls, "_p", "J");

  if (!fid)
  {
    qiLogFatal() << "JNIObject: Cannot get GenericObject";
    throw "JNIObject: Cannot get GenericObject";
  }

  jlong fieldValue = _env->GetLongField(_obj, fid);
  return *(reinterpret_cast<qi::ObjectPtr*>(fieldValue));
}

qi::GenericObject* JNIObject::genericObject()
{
  jfieldID fid = _env->GetFieldID(_cls, "_p", "J");

  if (!fid)
  {
    qiLogFatal() << "JNIObject: Cannot get GenericObject";
    throw "JNIObject: Cannot get GenericObject";
  }

  jlong fieldValue = _env->GetLongField(_obj, fid);
  qi::ObjectPtr* ptr = reinterpret_cast<qi::ObjectPtr*>(fieldValue);

  return reinterpret_cast<qi::GenericObject*>((*ptr).get());
}

void JNIObject::build(qi::ObjectPtr *newO)
{
  JVM()->GetEnv((void**) &_env, QI_JNI_MIN_VERSION);

  _cls = _env->FindClass(QI_OBJECT_CLASS);
  jmethodID mid = _env->GetMethodID(_cls, "<init>", "(J)V");

  if (!mid)
  {
    qiLogError() << "JNIObject : Cannot find constructor";
    throwJavaError(_env, "JNIObject : Cannot find constructor");
  }

  jlong pObj = (long) newO;

  _obj = _env->NewObject(_cls, mid, pObj);

  // Keep a global ref on this object to avoid destruction EVER
  _env->NewGlobalRef(_obj);
}
