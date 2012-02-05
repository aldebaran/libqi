/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_BROKER_H_
#define _QIMESSAGING_BROKER_H_

#include <qimessaging/context.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct {} qi_broker_t;


  /** \brief create a qi brokery.
   *  \return a pointer to an opaque structure, or NULL on error.
   *  \ingroup qiCapi
   */
  qi_client_t *qi_broker_create(const char *name);

  /** \brief destroy a qi broker.
   *  \param broker the broker to destroy.
   *  \ingroup qiCapi
   */
  void qi_broker_destroy(qi_broker_t *client);

  int qi_broker_connect(qi_client_t *client, const char *address);

  //TODO: or a custom struct with a function to destroy it
  int qi_broker_machines(qi_broker_t *client, qi_message_t *reply);
  //TODO: or a custom struct with a function to destroy it
  int qi_broker_services(qi_broker_t *client, qi_message_t *reply);

  //create a qi_client_t
  qi_client_t *qi_broker_create_client(qi_broker_t *client, const char *name);


#ifdef __cplusplus
}
#endif

#endif  // _QIMESSAGING_CLIENT_H_
