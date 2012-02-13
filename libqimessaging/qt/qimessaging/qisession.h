/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	QITRANSPORTSESSION_H_
# define   	QITRANSPORTSESSION_H_

#include <QtCore/QObject>
#include <QVector>
#include <QString>

class QiSessionPrivate;

class QiSession : public QObject {
  Q_OBJECT

public:
  QiSession();
  ~QiSession();

public:
  void connect(const QString &masterAddress);

  bool waitForConnected(int msecs = 30000);
  bool waitForDisconnected(int msecs = 30000);

  QObject           *service(const QString &name, const QString &type = "tcp");
  QVector<QString>   services();

signals:
  void OnConnected();
  void OnDisconnected();

  //really?
  void writeReady();
  void readReady();

protected:
  QiSessionPrivate *_p;
};


#endif 	    /* !QITRANSPORTSESSION_H_ */
