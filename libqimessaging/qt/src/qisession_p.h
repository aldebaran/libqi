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
};

class QiSessionPrivate : public qi::TransportSocketInterface {
public:
  virtual void onConnected(qi::TransportSocket *client);
  virtual void onDisconnected(qi::TransportSocket *client);
  virtual void onWriteDone(qi::TransportSocket *client);
  virtual void onReadyRead(qi::TransportSocket *client, int id);

  void service_end(qi::TransportSocket *client, qi::Message *msg, ServiceRequest &sr);
  //void services_end(qi::TransportSocket *client, int id, QFutureInterface<QObject *> *fu);

  qi::Session               session;
  QMap<int, ServiceRequest> _futureService;
};


#endif 	    /* !QISESSION_P_H_ */
