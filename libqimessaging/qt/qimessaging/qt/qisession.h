/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**  - Laurent LEC   <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	QT_QITRANSPORTSESSION_H_
# define   	QT_QITRANSPORTSESSION_H_

#include <qimessaging/api.hpp>
#include <qimessaging/service_info.hpp>
#include <qimessaging/url.hpp>
#include <QtCore/QObject>
#include <QtCore/QMetaObject>
#include <QtCore/QUrl>
#include <QVector>
#include <QString>

class QiSessionPrivate;

class QIMESSAGING_API QiSession : public QObject {
  Q_OBJECT

public:
  QiSession();
  ~QiSession();

public:
  void                     connect(const QString &masterAddress);
  void                     disconnect();
  bool                     waitForConnected(int msecs = 30000);
  bool                     waitForDisconnected(int msecs = 30000);
  QObject                 *service(const QString     &name,
                                   qi::Url::Protocol type = qi::Url::Protocol_Any);
  QVector<qi::ServiceInfo> services();

  // private implementation
  QiSessionPrivate *_p;

signals:
  void connected();
  void disconnected();
};


#endif 	    /* !QITRANSPORTSESSION_H_ */
