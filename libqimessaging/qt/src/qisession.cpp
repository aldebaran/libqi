/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <QString>
#include <qimessaging/qisession.h>

#include <qimessaging/session.hpp>

class QiSessionPrivate {
public:
  qi::Session session;
};

class QRemoteObject : public QObject {
public:
  explicit QRemoteObject(qi::TransportSocket *ts, const std::string &dest)
    : _ts(ts),
      _dest(dest)
  {
  }

  virtual void metaCall(const std::string &method, const std::string &sig, qi::DataStream &in, qi::DataStream &out) {
    qi::Message msg;
    msg.setId(200);
    msg.setSource("ouame");
    msg.setDestination(_dest);
    msg.setPath(method);
    msg.setData(in.str());
    _ts->send(msg);
    _ts->waitForId(msg.id());

    qi::Message ret;
    _ts->read(msg.id(), &ret);
    out.str(ret.data());
  }

protected:
  qi::TransportSocket *_ts;
  std::string _dest;
};



QiSession::QiSession()
  : _p(new QiSessionPrivate)
{
}

QiSession::~QiSession()
{
  delete _p;
}


void QiSession::connect(const QString &masterAddress)
{
  _p->session.connect(masterAddress.toUtf8().constData());
}

bool QiSession::waitForConnected(int msecs)
{
  return _p->session.waitForConnected(msecs);
}

bool QiSession::waitForDisconnected(int msecs)
{
  return _p->session.waitForDisconnected(msecs);
}

QObject *QiSession::service(const QString &name, const QString &type)
{
  qi::TransportSocket *ts = _p->session.serviceSocket(name.toUtf8().constData(), type.toUtf8().constData());
  QRemoteObject *robj = new QRemoteObject(ts, name.toUtf8().constData());
  QObject *obj = robj;
  return obj;
}

QVector<QString> QiSession::services()
{
  QVector<QString> ret;

  foreach (std::string str, _p->session.services())
    ret.push_back(QString::fromStdString(str));

  return ret;
}
