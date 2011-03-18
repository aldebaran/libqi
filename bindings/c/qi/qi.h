/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef         _QI_H_
# define        _QI_H_

#include <qi/context.h>
#include <qi/signature.h>
#include <qi/message.h>
#include <qi/client.h>
#include <qi/server.h>

#ifdef __cplusplus
extern "C"
{
#endif

  // MASTER API
  char *qi_master_locate_service(qi_client_t *client, const char *signature);

#ifdef __cplusplus
}
#endif


#endif /* !_QI_H_ */
