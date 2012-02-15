/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	QT_QISERVER_H_
# define   	QT_QISERVER_H_

#include <QtCore/QObject>
#include <qimessaging/qisession.h>

class QiServerPrivate;
class NetworkThread;

class QiServer {
public:
  QiServer();
  ~QiServer();

  void listen(QiSession *session, const QUrl &url);
  void stop();

  void advertiseService(const QString &name, QObject *obj);

protected:
  QiServerPrivate *_p;
};


#endif 	    /* !QISERVER_H_ */
