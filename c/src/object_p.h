/*
**
** Author(s):
**  - Julien Freche <jfreche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef	_QIMESSAGING_OBJECT_C_P_H_
# define _QIMESSAGING_OBJECT_C_P_H_

#include <qitype/genericobject.hpp>

inline qi::AnyObject &qi_object_cpp(qi_object_t *value) {
  qi::AnyObject &o = *(reinterpret_cast<qi::AnyObject *>(value));
  return o;
};

inline qi_object_t*         qi_object_create_from(qi::AnyObject obj) {
  qi::AnyObject *o = new qi::AnyObject(obj);
  return (qi_object_t *)o;
}


#endif /* !_QIMESSAGING_OBJECT_C_P_H_ */
