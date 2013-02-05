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
#include <qimessaging/c/object_c.h>

#include "objectbuilder_jni.hpp"

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderCreate()
{
  return (jlong) qi_object_builder_create();
}

jlong   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderGetObject(JNIEnv *env, jobject jobj, jlong pObjectBuilder)
{
  return (jlong) qi_object_builder_get_object((qi_object_builder_t *) pObjectBuilder);
}

void   Java_com_aldebaran_qimessaging_GenericObject_qiObjectBuilderDestroy(long pObjectBuilder)
{
  qi_object_builder_destroy((qi_object_builder_t *)pObjectBuilder);
}
