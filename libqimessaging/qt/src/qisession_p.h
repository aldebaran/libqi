/*
** Author(s):
**  - Laurent Lec  <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	QISESSION_P_H_
# define   	QISESSION_P_H_

#include <QtCore/qobject.h>
#include <QtCore/qmap.h>
#include <qimessaging/session.hpp>
#include <qimessaging/transport_socket.hpp>

struct ServiceRequest {
  QFutureInterface<QObject *> fu;
  QString                     name;
  unsigned int                serviceId;
  qi::Url::Protocol           protocol;
};

namespace qi {
  class NetworkThread;
};

class QiSessionPrivate : public qi::TransportSocketInterface {
public:
  QiSessionPrivate();
  ~QiSessionPrivate();

  virtual void onSocketConnected(qi::TransportSocket *client);
  virtual void onSocketReadyRead(qi::TransportSocket *client, int id);
  virtual void onSocketConnectionError(qi::TransportSocket *client);

  void service_ep_end(int id, qi::TransportSocket *client, qi::Message *msg, ServiceRequest &sr);
  void service_mo_end(int id, qi::TransportSocket *client, qi::Message *msg, ServiceRequest &sr);
  void services_end(qi::TransportSocket *client, qi::Message *msg, QFutureInterface< QVector<qi::ServiceInfo> > &fut);

public:
  qi::TransportSocket                                     *_serviceSocket;
  qi::NetworkThread                                       *_networkThread;

  QMap<int, ServiceRequest>                                _futureService;
  QMap<void *, ServiceRequest>                             _futureConnect;
  QMap<int, QFutureInterface< QVector<qi::ServiceInfo> > > _futureServices;
};


#endif 	    /* !QISESSION_P_H_ */
