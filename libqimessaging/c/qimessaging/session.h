/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_BROKER_H_
#define _QIMESSAGING_BROKER_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct {} qi_session_t;

  /** \brief create a qi connection
   *
   * \return a pointer to an opaque structure, or NULL on error.
   * \ingroup qiCapi
   */
  qi_session_t *qi_session_create();

  /** \brief connect a qi connection
   *
   * \param session session to connect
   * \param address address to connect to
   * \ingroup qiCapi
   */
  void qi_session_connect(qi_session_t *session,
                          const char   *address);

  /** \brief wait for the connection completion
   *
   * \param session the connecting session
   * \param msecs waiting timeout
   * \ingroup qiCapi
   */
  void qi_session_wait_for_connected(qi_session_t *session,
                                     int           msecs);

  /** \brief get a service
   *
   * \param session session
   * \param name name of the service
   * \return a pointer to related object, or NULL on error.
   * \ingroup qiCapi
   */
  qi_object_t *qi_session_get_service(qi_session_t *session,
                                      const char   *name);

  /** \brief destroy a qi connection
   *
   * \param session to destroy
   * \ingroup qiCapi
   */
  void qi_session_destroy(qi_session_t *session);

  /** \brief disconnect a qi connection
   *
   * \param session the session to disconnect
   * \ingroup qiCapi
   */
  void qi_session_disconnect(qi_session_t *session);

  /** \brief wait for the disconnection completion
   *
   * \param session disconnecting session
   * \param msecs waiting timeout
   * \ingroup qiCapi
   */
  void qi_session_for_disconnected(qi_session_t *session,
                                   int           msecs);
  /* \brief retrieve the list of available services
   *
   * \note The returned list is an array of char* terminated
   * by a NULL pointer. The list must be freed using
   * qi_session_free_services_list.
   *
   * \param session session from which the list is retrieved
   * \return list of services names
   * \ingroup qiCapi
   */

  const char** qi_session_get_services(qi_session_t *session);

  /* \brief Free a list retrieved with qi_session_get_services
   * \param session associated session
   * \param list list to free
   * \ingroup qiCapi
   */
  void qi_session_free_services_list(qi_session_t *session,
                                     const char  **list);

#ifdef __cplusplus
}
#endif

#endif  // _QIMESSAGING_CLIENT_H_
