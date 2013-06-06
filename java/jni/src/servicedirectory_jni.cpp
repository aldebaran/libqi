/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/log.hpp>
#include <qimessaging/servicedirectory.hpp>
#include "jnitools.hpp"
#include "servicedirectory_jni.hpp"

jlong   Java_com_aldebaran_qimessaging_ServiceDirectory_qiTestSDCreate(JNIEnv *env, jobject obj)
{
  qi::ServiceDirectory *sd = new qi::ServiceDirectory();

  qi::Future<void> fut = sd->listen("tcp://0.0.0.0:0");
  fut.wait();
  if (fut.hasError())
  {
    std::stringstream ss;

    ss << "Cannot get test Service Directory: " << fut.error();
    qiLogError("qimessaging.jni") << ss.str();
    delete sd;
    throwJavaError(env, ss.str().c_str());
    return (jlong) 0;
  }

  return (jlong) sd;
}

void    Java_com_aldebaran_qimessaging_ServiceDirectory_qiTestSDDestroy(jlong pSD)
{
  qi::ServiceDirectory *sd = reinterpret_cast<qi::ServiceDirectory *>(pSD);

  delete sd;
}

jstring Java_com_aldebaran_qimessaging_ServiceDirectory_qiListenUrl(JNIEnv* QI_UNUSED(env), jobject QI_UNUSED(obj), jlong pSD)
{
  qi::ServiceDirectory *sd = reinterpret_cast<qi::ServiceDirectory *>(pSD);

  if (!sd)
  {
    qiLogError("qimessaging.jni")  << "ServiceDirectory doesn't exist.";
    return 0;
  }

  return qi::jni::toJstring(sd->endpoints().at(0).str());
}

void    Java_com_aldebaran_qimessaging_ServiceDirectory_qiTestSDClose(jlong pSD)
{
  qi::ServiceDirectory *sd = reinterpret_cast<qi::ServiceDirectory *>(pSD);

  if (!sd)
  {
    qiLogError("qimessaging.jni") << "Reference to Service Directory is null.";
    return;
  }

  sd->close();
}
