/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_APPLICATION_H_
#define _QIMESSAGING_APPLICATION_H_

#include <qimessaging/c/api_c.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct qi_application_t_s {} qi_application_t;

  QIMESSAGING_API qi_application_t *qi_application_create(int ac, char **av);
  QIMESSAGING_API void              qi_application_destroy(qi_application_t *app);
  QIMESSAGING_API void              qi_application_run(qi_application_t *app);

#ifdef __cplusplus
}
#endif

#endif // ! _QIMESSAGING_APPLICATION_H_