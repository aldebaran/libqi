/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	QT_QISERVER_H_
# define   	QT_QISERVER_H_

# include <QtCore/qobject.h>
# include <QtCore/qurl.h>
# include <QtCore/qlist.h>
# include <qimessaging/qt/api.h>
# include <qimessaging/qt/qisession.h>

class QiServerPrivate;
class NetworkThread;

class QIMESSAGING_QT_API QiServer {
public:
  QiServer();
  ~QiServer();

  bool listen(QiSession *session, const QUrl &url);
  void close();

  QFuture<unsigned int> registerService(const QString &name,
                                        QObject *obj);
  //QFuture<void>         unregisterService(unsigned int idx);

  //QList<qi::ServiceInfo> registeredServices();
  //qi::ServiceInfo        registeredService(const QString &name);
  //QObject               *registeredServiceObject(const QString &service);

  QUrl                  listenUrl() const;

protected:
  QiServerPrivate *_p;
};


#endif 	    /* !QISERVER_H_ */
