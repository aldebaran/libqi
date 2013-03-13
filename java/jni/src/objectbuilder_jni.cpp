/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <map>
#include <qi/log.hpp>
#include <qitype/metamethod.hpp>
#include <qitype/genericobjectbuilder.hpp>

#include "objectbuilder_jni.hpp"

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderCreate()
{
  qi::GenericObjectBuilder *ob = new qi::GenericObjectBuilder();
  return (jlong) ob;
}

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderGetObject(JNIEnv *env, jobject jobj, jlong pObjectBuilder)
{
  qi::GenericObjectBuilder *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  qi::ObjectPtr *obj = new qi::ObjectPtr();
  qi::ObjectPtr &o = *(reinterpret_cast<qi::ObjectPtr *>(obj));

  o = ob->object();
  return (jlong) obj;
}

void   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderDestroy(long pObjectBuilder)
{
  qi::GenericObjectBuilder *ob = reinterpret_cast<qi::GenericObjectBuilder *>(pObjectBuilder);
  delete ob;
}
