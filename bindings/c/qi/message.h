/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef         _QI_MESSAGE_H_
# define        _QI_MESSAGE_H_

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void qi_message_t;


  qi_message_t *qi_message_create();
  void          qi_message_destroy();

  void          qi_message_write_bool(qi_message_t   *msg, const char b);
  void          qi_message_write_char(qi_message_t   *msg, const char b);
  void          qi_message_write_int(qi_message_t    *msg, const int i);
  void          qi_message_write_float(qi_message_t  *msg, const float f);
  void          qi_message_write_double(qi_message_t *msg, const double d);
  void          qi_message_write_string(qi_message_t *msg, const char *);
  void          qi_message_write_raw(qi_message_t    *msg, const char *, unsigned int size);

  char   qi_message_read_bool(qi_message_t   *msg);
  char   qi_message_read_char(qi_message_t   *msg);
  int    qi_message_read_int(qi_message_t    *msg);
  float  qi_message_read_float(qi_message_t  *msg);
  double qi_message_read_double(qi_message_t *msg);
  char  *qi_message_read_string(qi_message_t *msg);
  char  *qi_message_read_raw(qi_message_t    *msg, unsigned int *size);


#ifdef __cplusplus
}
#endif

#endif /* !_QI_H_ */
