/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef         _QI_CONTEXT_H_
# define        _QI_CONTEXT_H_

#ifdef __cplusplus
extern "C"
{
#endif

  typedef void qi_context_t;

  qi_context_t *qi_context_create();
  void          qi_context_destroy(qi_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* !_QI_H_ */
