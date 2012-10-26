/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _PYTHON_SRC_SD_TESTWRAP_
#define _PYTHON_SRC_SD_TESTWRAP_

#include <Python.h>
#include <qimessaging/c/object_c.h>

/*!
 * \brief The servicedirectory class
 * Quickly wrap service directory for automatic test purpose.
 */
class servicedirectory
{
public:
  servicedirectory();
  ~servicedirectory();
  char *listen_url();
  void  close();

private:
  void* _sd;
};

#endif // !_PYTHON_SRC_SD_TESTWRAP_
