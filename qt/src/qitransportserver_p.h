/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef  QITRANSPORTSERVER_P_H_
# define QITRANSPORTSERVER_P_H_

# include <qimessaging/qt/QiTransportServer>

# include <QTcpServer>
# include <QQueue>

class QTcpServerSsl : public QTcpServer
{
  Q_OBJECT

public:
  QTcpSocket* nextPendingConnection();
  virtual void incomingConnection(int sd);

  QQueue<QTcpSocket*> _pendingConnections;
  bool                _ssl;
  QByteArray          _key;
  QByteArray          _certificate;
};


class QiTransportServerPrivate : public QObject
{
  Q_OBJECT

public:
  QiTransportServerPrivate(QObject* parent,
                           QiTransportServer* self);
  virtual ~QiTransportServerPrivate();

  QiTransportServer         *_self;
  QTcpServerSsl              _server; // also handles non-encrypted TCP
  QQueue<QiTransportSocket*> _pendingConnections;
  QUrl                       _listeningUrl;

public slots:
  void acceptConnection();

signals:
  void newConnection();
};

#endif /* ifndef QITRANSPORTSERVER_P_H_ */
