/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef  QIGATEWAY_P_H_
# define QIGATEWAY_P_H_

# include <qimessaging/qt/QiGateway>
# include <qimessaging/qt/QiTransportServer>

# include <QMap>

class QiGatewayPrivate : public QObject
{
  Q_OBJECT

public:
  enum Type
  {
    Type_Local   = 1,
    Type_Reverse = 2,
    Type_Remote  = 3,
  };

  QiGatewayPrivate(QObject* parent, Type type);
  ~QiGatewayPrivate();

  void attachToServiceDirectory(const QUrl& address);
  bool listen(const QUrl& address);
  void close();

private:
  struct RemoteService
  {
    QiTransportSocket*    socket;
    unsigned int          serviceId;
    QString               serviceName;
    QList<QPair<QiTransportSocket*, const qi::Message*> >    pendingMessages;
  };

  void processClientMessage(QiTransportSocket* socket,
                            const qi::Message* msg);
  void processServiceMessage(QiTransportSocket* socket,
                             const qi::Message& msg);
  void connectToService(unsigned int serviceId,
                        const QUrl&  url);
  void routeMessage(QiTransportSocket* src,
                    QiTransportSocket* dst,
                    const qi::Message& msg);

private slots:
  void socketConnected();
  void newConnection();
  void socketDisconnected();
  void socketReadyRead();

private:
  Type                                    _type;
  QiTransportServer                       _server;
  QMap<unsigned int, RemoteService>       _services;
  QMap<QiTransportSocket*, QMap<unsigned int, QPair<unsigned int, QiTransportSocket*> > > _routingTable;
  QList<QiTransportSocket*>               _sockets;
};

#endif /* ifndef QIGATEWAY_P_H_ */
