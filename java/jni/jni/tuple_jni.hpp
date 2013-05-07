/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_TUPLE_HPP_
#define _JAVA_JNI_TUPLE_HPP_

#include <jni.h>

class JNITuple
{
  public:
    JNITuple(jobject obj);
    JNITuple(int size);
    ~JNITuple();

    int size();
    jobject get(int index);
    void set(int index, jobject obj);
    jobject object();

  private:

    jobject _obj;
    jclass  _cls;
    JNIEnv* _env;
};

#endif // !_JAVA_JNI_TUPLE_HPP_
