/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	QIREMOTEOBJECT_P_H_
# define   	QIREMOTEOBJECT_P_H_

#include <QtCore/qobject.h>

namespace qi {
  class TransportSocket;
  class MetaObject;
};

class QiRemoteObjectPrivate;

class QiRemoteObject : public QObject {
  //DO NOT ADD Q_OBJECT HERE

public:
  explicit QiRemoteObject(qi::TransportSocket *ts, const std::string &dest, unsigned int serviceId, const qi::MetaObject &metaobject);
  virtual ~QiRemoteObject();

  // provide custom Q_OBJECT implementation
  virtual const QMetaObject* metaObject() const;
  virtual int qt_metacall(QMetaObject::Call c, int id, void **a);
  virtual void *qt_metacast(const char* className);

protected:
  QiRemoteObjectPrivate *_p;
};

#endif 	    /* !QIREMOTEOBJECT_P_H_ */
