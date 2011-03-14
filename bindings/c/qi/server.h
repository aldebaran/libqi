/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef         _QI_SERVER_H_
# define        _QI_SERVER_H_

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void qi_server_t;
  typedef void (*BoundMethod)(qi_message_t *msg, qi_message_t *ret, void *data);

  qi_server_t *qi_server_create(const char *name);
  void         qi_server_destroy(qi_server_t *server);
  void         qi_server_connect(qi_server_t *server, const char *address);

  void         qi_server_advertise_service(qi_server_t *server  , const char *methodSignature, BoundMethod func, void *data);
  void         qi_server_unadvertise_service(qi_server_t *server, const char *methodSignature);

#ifdef __cplusplus
}
#endif

#endif /* !_QI_H_ */
