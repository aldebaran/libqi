/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_LIST_HPP_
#define _JAVA_JNI_LIST_HPP_

#include <jni.h>

class JNIList
{
  public:
    JNIList(jobject obj);
    JNIList();
    ~JNIList();

    int size();
    jobject get(int index);
    jobject object();
    bool    push_back(jobject current);

  private:
    jobject _obj;
    JNIEnv* _env;
    jclass  _cls;
};

#endif // !_JAVA_JNI_LIST_HPP_
