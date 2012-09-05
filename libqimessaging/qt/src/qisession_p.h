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
  QString                     protocol;
};

namespace qi {
  class NetworkThread;
};

class QiSessionPrivate : public qi::TransportSocketInterface,
                         public qi::SessionInterface
{
public:
  QiSessionPrivate(QiSession *self);
  ~QiSessionPrivate();

  virtual void onSocketConnected(qi::TransportSocket *client, void *data);
  virtual void onSocketReadyRead(qi::TransportSocket *client, int id, void *data);
  virtual void onSocketConnectionError(qi::TransportSocket *client, void *data);

  //Session
  virtual void onServiceRegistered(qi::Session *QI_UNUSED(session),
                                   const std::string &serviceName,
                                   void *data);
  virtual void onServiceUnregistered(qi::Session *QI_UNUSED(session),
                                     const std::string &serviceName,
                                     void *data);

  void service_ep_end(int id, qi::TransportSocket *client, qi::Message *msg, ServiceRequest &sr);
  void service_mo_end(int id, qi::TransportSocket *client, qi::Message *msg, ServiceRequest &sr);
  void services_end(qi::TransportSocket *client, qi::Message *msg, QFutureInterface< QVector<qi::ServiceInfo> > &fut);

public:
  qi::TransportSocket                                     *_serviceSocket;
  qi::Session                                             *_session;
  QiSession                                               *_self;

  QMap<int, ServiceRequest>                                _futureService;
  QMap<void *, ServiceRequest>                             _futureConnect;
  QMap<int, QFutureInterface< QVector<qi::ServiceInfo> > > _futureServices;
};


#endif 	    /* !QISESSION_P_H_ */
