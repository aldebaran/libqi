/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   QIREMOTEOBJECT_P_H_
# define  QIREMOTEOBJECT_P_H_

#include <QtCore/qobject.h>
#include <boost/shared_ptr.hpp>

namespace qi {
  class TransportSocket;
  typedef boost::shared_ptr<TransportSocket> TransportSocketPtr;
  class MetaObject;
};

class QiRemoteObjectPrivate;

class QiRemoteObject : public QObject {
  //DO NOT ADD Q_OBJECT HERE

public:
  explicit QiRemoteObject(qi::TransportSocketPtr ts, const std::string &dest, unsigned int serviceId, const qi::MetaObject &metaobject);
  virtual ~QiRemoteObject();

  // provide custom Q_OBJECT implementation
  virtual const QMetaObject* metaObject() const;
  virtual int qt_metacall(QMetaObject::Call c, int id, void **a);
  virtual void *qt_metacast(const char* className);

protected:
  QiRemoteObjectPrivate *_p;
};

#endif  /* !QIREMOTEOBJECT_P_H_ */
