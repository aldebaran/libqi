/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_SESSION_H_
#define _QIMESSAGING_SESSION_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

  typedef struct qi_session_t_s {} qi_session_t;

  //forward declaration
  typedef struct qi_object_t_s     qi_object_t;

  /** \brief create a qi connection
   *
   * \return a pointer to an opaque structure, or NULL on error.
   * \ingroup qiCapi
   */
  QIMESSAGING_API qi_session_t *qi_session_create();

  /** \brief connect a qi connection
   *
   * \param session session to connect
   * \param address address to connect to
   * \ingroup qiCapi
   */ // P.R. FIXME: make asynchronous, return a future, make a _sync version
  QIMESSAGING_API void qi_session_connect(qi_session_t *session, const char   *address);

  /* \brief active server side of session
   * \param listening address
   */
  QIMESSAGING_API bool qi_session_listen(qi_session_t *session, const char *address);

  /* \brief expose an object to the world
   * \param name of the module
   * \param object to register
   */
  QIMESSAGING_API int qi_session_register_service(qi_session_t *session, const char *name, qi_object_t *object);

  /* \brief hide a object previously exposed to the world
   * \param object to unregister
   */
  QIMESSAGING_API void qi_session_unregister_service(qi_session_t *session, unsigned int idx);

  /** \brief get a service
   *
   * \param session session
   * \param name name of the service
   * \return a pointer to related object, or NULL on error.
   * \ingroup qiCapi
   */ // P.R. FIXME: make asynchronous, return a future, make a _sync version
  QIMESSAGING_API qi_object_t *qi_session_get_service(qi_session_t *session, const char *name);

  /** \brief destroy a qi connection
   *
   * \param session to destroy
   * \ingroup qiCapi
   */
  QIMESSAGING_API void qi_session_destroy(qi_session_t *session);

  /** \brief disconnect a qi connection
   *
   * \param session the session to disconnect
   * \ingroup qiCapi
   */
  QIMESSAGING_API void qi_session_close(qi_session_t *session);

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

  QIMESSAGING_API const char** qi_session_get_services(qi_session_t *session);

  /* \brief Free a list retrieved with qi_session_get_services
   * \param session associated session
   * \param list list to free
   * \ingroup qiCapi
   */
  QIMESSAGING_API void qi_session_free_services_list(const char **list);


#ifdef __cplusplus
}
#endif

#endif  // _QIMESSAGING_CLIENT_H_
