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

  if (sd->listen("tcp://0.0.0.0:0") == false)
  {
    qiLogError("qimessaging.jni") << "Cannot get test Service Directory";
    delete sd;
    throwJavaError(env, "Cannot get test ServiceDirectory");
    return (jlong) 0;
  }

  return (jlong) sd;
}

void    Java_com_aldebaran_qimessaging_ServiceDirectory_qiTestSDDestroy(jlong pSD)
{
  qi::ServiceDirectory *sd = reinterpret_cast<qi::ServiceDirectory *>(pSD);

  delete sd;
}

jstring Java_com_aldebaran_qimessaging_ServiceDirectory_qiListenUrl(JNIEnv *env, jobject obj, jlong pSD)
{
  qi::ServiceDirectory *sd = reinterpret_cast<qi::ServiceDirectory *>(pSD);

  if (!sd)
  {
    qiLogError("qimessaging.jni")  << "ServiceDirectory doesn't exist.";
    return 0;
  }

  return toJavaString(env, sd->endpoints().at(0).str());
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
