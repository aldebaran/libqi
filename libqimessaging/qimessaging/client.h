/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef         _QI_CLIENT_H_
# define        _QI_CLIENT_H_



#ifdef __cplusplus
extern "C"
{
#endif

  typedef void qi_client_t;


  /** \brief create a qi client.
   *  \param name client's name. only used for display
   *  \return a pointer to an opaque structure, or NULL on error.
   *  \ingroup qiCapi
   */
  qi_client_t *qi_client_create(const char *name);

  /** \brief create a qi client with a context
   *  \param name client's name. only used for display
   *  \return a pointer to an opaque structure, or NULL on error.
   *  \ingroup qiCapi
   */
  qi_client_t *qi_client_create_with_context(const char *name, qi_context_t *ctx);

  /** \brief destroy a qi client.
   *  \param client the client to destroy.
   *  \ingroup qiCapi
   */
  void qi_client_destroy(qi_client_t *client);

  /** \brief connect a client to a master
   *  this function do not block.
   *
   *  \param client the client to destroy.
   *  \param address address of the master
   *  \return 0 on success, 1 on failure
   *  \ingroup qiCapi
   */
  int qi_client_connect(qi_client_t *client, const char *address);

  /** \brief call a method on the client.
   * This will invoke the method using the more appropriate transport.
   * it could be local, interprocess or remote. This function will wait
   * for the result from the client and return it in ret.
   *
   * \param client the client to destroy.
   * \param method the signature of the method to call
   * \param request message to send
   * \param reply message returned in response
   * \return 0 on success, 1 on failure
   * \ingroup qiCapi
   */
  int qi_client_call(qi_client_t *client, const char *method, qi_message_t *request, qi_message_t *reply);

  typedef void (*SubscriberFunction)(const char *, qi_message_t *, void *);
  int qi_client_subscribe(qi_client_t *client, const char *method_signature, SubscriberFunction func, void *data);
  int qi_client_unsubscribe(const char *method_signature);


#ifdef __cplusplus
}
#endif

#endif /* !_QI_H_ */
