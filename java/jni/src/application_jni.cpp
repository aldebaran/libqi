/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <cstring>
#include <qimessaging/c/application_c.h>
#include "application_jni.hpp"

jlong Java_com_aldebaran_qimessaging_Application_qiApplicationCreate()
{
  // Emulate empty command line arguments.
  int argc = 1;
  char **argv = new char*[2];

  // Fill first argument with random name. FIXME : seriously....
  argv[0] = new char[10];
  ::strcpy(argv[0], "java");

  // Call qi_application_create
  jlong ret = (jlong) qi_application_create(&argc, argv);

  delete[] argv[0];
  delete[] argv;
  return ret;
}

void Java_com_aldebaran_qimessaging_Application_qiApplicationDestroy(jlong pApplication)
{
  qi_application_destroy(reinterpret_cast<qi_application_t *>(pApplication));
}

void Java_com_aldebaran_qimessaging_Application_qiApplicationRun(jlong pApplication)
{
  qi_application_run(reinterpret_cast<qi_application_t *>(pApplication));
}
