/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_ENUMERATION_HPP_
#define _JAVA_JNI_ENUMERATION_HPP_

#include <jni.h>

class JNIEnumeration
{
  public:
    JNIEnumeration(jobject obj);
    ~JNIEnumeration();

    bool hasNextElement();
    jobject nextElement();

  private:

    jobject _obj;
    jclass  _cls;
    JNIEnv* _env;
};

#endif // !_JAVA_JNI_ENUMERATION_HPP_
