/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef QITRANSPORTSOCKET_H_
# define QITRANSPORTSOCKET_H_

# include <qimessaging/qt/api.h>
# include <qimessaging/message.hpp>
# include <QAbstractSocket>
# include <QUrl>

class QiTransportSocketPrivate;

class QIMESSAGING_QT_API QiTransportSocket : public QObject
{
  Q_OBJECT

public:
  QiTransportSocket(QObject* parent = 0);
  ~QiTransportSocket();

  void write(const qi::Message& message);
  qi::Message* read();
  void close();
  void connectToHost(const QUrl& address);
  QUrl peer() const;

  enum SocketState
  {
    SocketState_Unconnected = QAbstractSocket::UnconnectedState,
    SocketState_HostLookup  = QAbstractSocket::HostLookupState,
    SocketState_Connecting  = QAbstractSocket::ConnectingState,
    SocketState_Connected   = QAbstractSocket::ConnectedState,
    SocketState_Bound       = QAbstractSocket::BoundState,
    SocketState_Listening   = QAbstractSocket::ListeningState,
    SocketState_Closing     = QAbstractSocket::ClosingState,
    SocketState_Unknown     = -1,
  };

  SocketState state() const;

signals:
  void readyRead();
  void connected();
  void disconnected();

public:
  QiTransportSocketPrivate* _p;
};

#endif /* ifndef QITRANSPORTSOCKET_H_ */
