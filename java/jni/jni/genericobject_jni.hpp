/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_GENERICOBJECT_HPP_
#define _JAVA_JNI_GENERICOBJECT_HPP_

#include <list>
#include <qimessaging/api.hpp>
#include <jni.h>

extern "C"
{
  QIMESSAGING_API jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectCreate();
  QIMESSAGING_API void    Java_com_aldebaran_qimessaging_GenericObject_qiObjectDestroy(JNIEnv *env, jobject jobj, jlong pObject);
  QIMESSAGING_API jobject Java_com_aldebaran_qimessaging_GenericObject_qiObjectCall(JNIEnv *env, jobject obj, jlong pObject, jstring jmethod, jobjectArray args);
  QIMESSAGING_API jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectRegisterMethod(JNIEnv *env, jobject obj, jlong pObjectBuilder, jstring method, jobject instance, jstring service);
} // !extern "C"

struct qi_method_info
{
  jobject     instance; // QimessagingService implementation instance
  std::string sig; // Complete signature
  jobject     jobj; // GenericObject Java instance
  std::string className; // Class specialisation of com.aldebaran.Qimessagingservice

  qi_method_info(jobject jinstance, const std::string& jsig, jobject object, std::string name)
  {
    instance = jinstance;
    sig = jsig;
    jobj = object;
    className = name;
  }
};

class MethodInfoHandler
{
  public:
  MethodInfoHandler() {}
  ~MethodInfoHandler(){}

  void push(qi_method_info *newInfo)
  {
    _infos.push_back(newInfo);
  }

  void pop(jobject jobj)
  {
    std::list<qi_method_info*>::iterator it;

    it = _infos.begin();
    while (_infos.empty() == false && it != _infos.end())
    {
      // Only delete method_info related to given object
      if ((*it)->jobj == jobj)
      {
        delete (*it);
        _infos.remove(*it);
        it = _infos.begin(); // iterator is fucked up, reset it.
      }
      ++it;
    }
  }

  private:
    std::list<qi_method_info*>  _infos;
};

#endif // !_JAVA_JNI_GENERICOBJECT_HPP_

