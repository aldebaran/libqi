/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012, 2013 Aldebaran Robotics
*/

#ifndef _JAVA_JNI_CALLBRIDGE_HPP_
#define _JAVA_JNI_CALLBRIDGE_HPP_

#include <list>
#include <jni.h>

#include <qimessaging/api.hpp>
#include <jnitools.hpp>

// Generic callback for call forward
qi::Future<qi::GenericValuePtr>*    call_from_java(JNIEnv *env, qi::ObjectPtr object, const std::string& strMethodName, jobjectArray listParams);
qi::GenericValuePtr                 call_to_java(std::string signature, void* data, const qi::GenericFunctionParameters& params);
qi::GenericValuePtr                 event_callback_to_java(void *vinfo, const std::vector<qi::GenericValuePtr>& params);

struct qi_method_info
{
  jobject     instance; // QimessagingService implementation instance
  std::string sig; // Complete signature
  jobject     jobj; // GenericObject Java instance
  std::string className; // Class specialisation of com.aldebaran.QimessagingService

  qi_method_info(jobject jinstance, const std::string& jsig, jobject object, std::string name)
  {
    instance = jinstance;
    sig = jsig;
    jobj = object;
    className = name;
  }

  ~qi_method_info()
  {
    JNIEnv* env = qi::jni::env();

    if (!env)
      return;

    env->DeleteGlobalRef(instance);
    env->DeleteGlobalRef(jobj);
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

#endif // !_JAVA_JNI_CALLBRIDGE_HPP_
