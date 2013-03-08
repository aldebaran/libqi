/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef	_QIMESSAGING_VALUE_C_P_H_
# define _QIMESSAGING_VALUE_C_P_H_

#include <qitype/genericvalue.hpp>

inline qi::GenericValuePtr &qi_value_cpp(qi_value_t *value) {
  qi::GenericValuePtr &gv = *(reinterpret_cast<qi::GenericValuePtr *>(value));
  return gv;
};

#endif /* !_QIMESSAGING_VALUE_C_P_H_ */
