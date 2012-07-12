/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Laurent LEC   <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	QT_QITRANSPORTSESSION_H_
# define   	QT_QITRANSPORTSESSION_H_

#include <qimessaging/qt/api.h>
#include <qimessaging/service_info.hpp>
#include <qimessaging/url.hpp>
#include <QtCore/qobject.h>
#include <QtCore/qfuture.h>
#include <QtCore/qvector.h>
#include <QtCore/qstring.h>

class QiSessionPrivate;

class QIMESSAGING_QT_API QiSession : public QObject {
  Q_OBJECT

public:
  QiSession();
  ~QiSession();

public:
  bool connect(const QString &masterAddress);
  bool disconnect();
  bool waitForConnected(int msecs = 30000);
  bool waitForDisconnected(int msecs = 30000);

  QFuture<QObject *>                  service(const QString     &name,
                                              const QString     &type = QString("any"));
  QFuture< QVector<qi::ServiceInfo> > services();

  // private implementation
  QiSessionPrivate *_p;

signals:
  void connected();
  void disconnected();
};


#endif 	    /* !QITRANSPORTSESSION_H_ */
