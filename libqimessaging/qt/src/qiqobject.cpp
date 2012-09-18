/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "qiqobject.h"
#include <qimessaging/genericobject.hpp>
#include <QtCore/qdebug.h>
#include <QtCore/QMetaMethod>

class QiObjectPrivate {
public:
};

QiObject::QiObject(QObject *object)
  : _p (new QiObjectPrivate)
{
  int i = 0;
  //construct a qi::MetaObject from the object
  int mc = object->metaObject()->methodCount();
  int mo = object->metaObject()->methodOffset();
  for (i = 0; i < mc; ++i) {
    QMetaMethod qmm = object->metaObject()->method(i + mo);
    qmm.signature();
  }
}


