/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <cstring>
#include <qi/log.hpp>
#include "error_p.h"
#include <qimessaging/c/error_c.h>

qiLogCategory("qimessaging.error");

// Internal qimessaging-c bindings error buffer.
static char* _qi_c_error = 0;
// Size of internal error buffer
#define _QI_C_ERROR_SIZE 100

static char* _qi_c_get_error_buffer()
{
  extern char* _qi_c_error;

  if (!_qi_c_error)
  {
    if (!(_qi_c_error = (char*) malloc(_QI_C_ERROR_SIZE)))
      qiLogError() << "Cannot allocate error buffer, you're gonna have a bad time.";
  }

  return _qi_c_error;
}

bool  qi_c_set_error(const std::string& error)
{
  char* cerror = _qi_c_get_error_buffer();

  if (error.size() > _QI_C_ERROR_SIZE)
    return false;

  ::memcpy(cerror, error.c_str(), error.size());
  cerror[error.size()] = 0;

  return true;
}

const char*  qi_c_error()
{
  extern char* _qi_c_error;

  return _qi_c_error;
}
