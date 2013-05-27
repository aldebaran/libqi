/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qi/future.hpp>

#include <qitype/genericobject.hpp>
#include <qitype/genericobjectbuilder.hpp>
#include <qitype/functiontype.hpp>

#include <callbridge.hpp>
#include <jobjectconverter.hpp>
#include <jnitools.hpp>

qiLogCategory("qimessaging.jni");

MethodInfoHandler gInfoHandler;

qi::Future<qi::GenericValuePtr>* call_from_java(JNIEnv *env, qi::ObjectPtr object, const std::string& strMethodName, jobjectArray listParams)
{
  qi::GenericFunctionParameters params;
  std::string signature;
  jsize size;
  jsize i = 0;
  bool  useSignature = true;

  size = env->GetArrayLength(listParams);
  while (i < size)
  {
    jobject current = env->NewGlobalRef(env->GetObjectArrayElement(listParams, i));
    qi::GenericValuePtr val = qi::GenericValueRef(current).clone();
    params.push_back(val);

    // In case of empty list or map, type system cannot resole containee type. In that case, do not use signature.
    if (val.signature(true).find(qi::Signature::Type_None) != std::string::npos)
      useSignature = false;

    signature += val.signature(true);
    i++;
  }

  // If user provide signature, do not use resolved one.
  if (strMethodName.find("::") != std::string::npos)
    useSignature = false;

  qi::Future<qi::GenericValuePtr> *fut = new qi::Future<qi::GenericValuePtr>();
  try
  {
    std::string nameWithOptionalSignature = strMethodName + (useSignature == true ? "::(" + signature + ")" : "");
    *fut = object->metaCall(nameWithOptionalSignature, params);
  } catch (std::runtime_error &e)
  {
    throwJavaError(env, e.what());
    return 0;
  }

  return fut;
}

qi::GenericValuePtr call_to_java(std::string signature, void* data, const qi::GenericFunctionParameters& params)
{
  qi::GenericValuePtr res;
  jvalue*             args = new jvalue[params.size()];
  int                 index = 0;
  JNIEnv*             env = 0;
  qi_method_info*     info = reinterpret_cast<qi_method_info*>(data);
  std::vector<std::string>  sigInfo = qi::signatureSplit(signature);

  // Attach JNIEnv to current thread to avoid segfault in jni functions. (eventloop dependent)
  if (JVM()->AttachCurrentThread((envPtr) &env, (void *) 0) != JNI_OK || env == 0)
  {
    qiLogError("qimessaging.java") << "Cannot attach callback thread to Java VM";
    throwJavaError(env, "Cannot attach callback thread to Java VM");
    return res;
  }

  // Check value of method info structure
  if (info == 0)
  {
    qiLogError("qimessaging.java") << "Internal method informations are not valid";
    throwJavaError(env, "Internal method informations are not valid");
    return res;
  }
  // Translate parameters from GenericValues to jobjects
  qi::GenericFunctionParameters::const_iterator it = params.begin();
  qi::GenericFunctionParameters::const_iterator end = params.end();
  for(; it != end; it++)
  {
    jvalue value;
    value.l = JObject_from_GenericValue(*it);
    args[index] = value;
    index++;
  }

  // Find method class and get methodID
  jclass cls = env->FindClass(info->className.c_str());
  jmethodID mid = env->GetMethodID(cls, sigInfo[1].c_str(), toJavaSignature(signature).c_str());

  if (!mid) // Then maybe method is declared as static...
    mid = env->GetStaticMethodID(cls, sigInfo[1].c_str(), toJavaSignature(signature).c_str());
  if (!mid)
  {
    qiLogError("qimessaging.jni") << "Cannot find java method " << sigInfo[1] << toJavaSignature(signature).c_str();
    throw new std::runtime_error("Cannot find method");
  }

  // Finally call method
  jobject ret = env->CallObjectMethodA(info->instance, mid, args);

  delete[] args;
  if (sigInfo[0] == "")
    return qi::GenericValuePtr(qi::typeOf<void>());

  return GenericValue_from_JObject(ret).first;
}

qi::GenericValuePtr event_callback_to_java(void *vinfo, const std::vector<qi::GenericValuePtr>& params)
{
  qi_method_info*  info = static_cast<qi_method_info*>(vinfo);

  qiLogVerbose("qimessaging.jni") << "Java event callback called (sig=" << info->sig << ")";

  return call_to_java(info->sig, info, params);
}
