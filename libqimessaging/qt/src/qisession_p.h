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
#include <qimessaging/transportsocket.hpp>

struct ServiceRequest {
  QFutureInterface<QObject *> fu;
  QString                     name;
  unsigned int                serviceId;
  QString                     protocol;
};

namespace qi {
  class NetworkThread;
};

class QiSessionPrivate
{
public:
  QiSessionPrivate(QiSession *self);
  ~QiSessionPrivate();

  void onSocketConnected(qi::TransportSocketPtr client);
  void onSocketDisconnected(qi::TransportSocketPtr client, int error);
  void onMessageReady(const qi::Message &msg, qi::TransportSocketPtr client);
  void onMessageTimeout(qi::TransportSocketPtr client, unsigned int id);

  //Session
  virtual void onServiceRegistered(qi::Session *QI_UNUSED(session),
                                   const std::string &serviceName,
                                   void *data);
  virtual void onServiceUnregistered(qi::Session *QI_UNUSED(session),
                                     const std::string &serviceName,
                                     void *data);

  void service_ep_end(int id, qi::TransportSocketPtr client, const qi::Message *msg, ServiceRequest &sr);
  void service_mo_end(int id, qi::TransportSocketPtr client, const qi::Message *msg, ServiceRequest &sr);
  void services_end(qi::TransportSocketPtr client, const qi::Message *msg, QFutureInterface< QVector<qi::ServiceInfo> > &fut);

public:
  qi::TransportSocketPtr                                   _serviceSocket;
  qi::Session                                             *_session;
  QiSession                                               *_self;

  QMap<int, ServiceRequest>                                _futureService;
  QMap<qi::TransportSocketPtr, ServiceRequest>             _futureConnect;
  QMap<int, QFutureInterface< QVector<qi::ServiceInfo> > > _futureServices;
};


#endif 	    /* !QISESSION_P_H_ */
