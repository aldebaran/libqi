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

  /*! \warning Must be the first thing called by main function. */
  QIMESSAGING_API qi_application_t *qi_application_create(int *argc, char **argv);
  QIMESSAGING_API void              qi_application_destroy(qi_application_t *app);
  QIMESSAGING_API void              qi_application_run(qi_application_t *app);
  QIMESSAGING_API void              qi_application_stop(qi_application_t *app);
  QIMESSAGING_API bool              qi_application_initialized();

#ifdef __cplusplus
}
#endif

#endif // ! _QIMESSAGING_APPLICATION_H_
