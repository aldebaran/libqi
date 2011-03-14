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

  qi_client_t *qi_client_create(const char *name);
  qi_client_t *qi_client_create_with_context(const char *name, qi_context_t *ctx);
  void         qi_client_destroy(qi_client_t *client);

  void         qi_client_connect(qi_client_t *client, const char *address);
  void         qi_client_call(qi_client_t *client, const char *method, qi_message_t *msg, qi_message_t *ret);

#ifdef __cplusplus
}
#endif

#endif /* !_QI_H_ */
