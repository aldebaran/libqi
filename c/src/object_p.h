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

inline qi::ObjectPtr &qi_object_cpp(qi_object_t *value) {
  qi::ObjectPtr &o = *(reinterpret_cast<qi::ObjectPtr *>(value));
  return o;
};

inline qi_object_t*         qi_object_create_from(qi::ObjectPtr obj) {
  qi::ObjectPtr *o = new qi::ObjectPtr(obj);
  return (qi_object_t *)o;
}


#endif /* !_QIMESSAGING_OBJECT_C_P_H_ */
