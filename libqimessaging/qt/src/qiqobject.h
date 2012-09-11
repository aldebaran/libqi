/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   QIREMOTEOBJECT_P_H_
# define  QIREMOTEOBJECT_P_H_

#include <qimessaging/object.hpp>
#include <QtCore/qobject.h>

class QiObjectPrivate;
class QiObject : public qi::Object {
public:
  QiObject(QObject *object);

protected:
  QiObjectPrivate *_p;
};

#endif  /* !QIREMOTEOBJECT_P_H_ */
