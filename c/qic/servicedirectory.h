/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2013 Aldebaran Robotics
*/

#ifndef   	SERVICEDIRECTORY_H_
# define   	SERVICEDIRECTORY_H_

#include <qic/api.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct qi_servicedirectory_t_s {} qi_servicedirectory_t;
  typedef struct qi_value_t_s qi_value_t;

  QIC_API qi_servicedirectory_t *qi_servicedirectory_create();
  QIC_API void                   qi_servicedirectory_destroy(qi_servicedirectory_t *app);
  QIC_API void                   qi_servicedirectory_listen(qi_servicedirectory_t *servicedirectory, const char *url);
  QIC_API void                   qi_servicedirectory_close(qi_servicedirectory_t *servicedirectory);
  QIC_API qi_value_t*            qi_servicedirectory_endpoints(qi_servicedirectory_t *servicedirectory);

#ifdef __cplusplus
}
#endif


#endif 	    /* !SERVICEDIRECTORY_H_ */
