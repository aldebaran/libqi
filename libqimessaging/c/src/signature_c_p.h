/*
**
** Author(s):
**  - Pierre ROULLON <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef     _QIMESSAGING_SIGNATURE_C_P_H_
# define    _QIMESSAGING_SIGNATURE_C_P_H_

# include <qitype/signature.hpp>

typedef struct qi_signature_t_s
{
  qi_signature_t_s(const char *signature) : _sig(signature) {}

  const char              *current;

  qi::Signature            _sig;
  qi::Signature::iterator  _it;
} qi_signature_t;


#endif // ! _QIMESSAGING_SIGNATURE_C_P_H_

