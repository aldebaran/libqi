/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include "qiremoteobject_p.h"
#include "qmetaobjectbuilder_p.h"

class QiRemoteObjectPrivate {
public:
  qi::TransportSocket *socket;
  std::string          dest;
  QMetaObject         *meta;
};


QiRemoteObject::QiRemoteObject(qi::TransportSocket *ts, const std::string &dest)
  : _p (new QiRemoteObjectPrivate)
{
  QMetaObjectBuilder mob;
  mob.setClassName("QiRemoteObject");
  //QMetaMethodBuilder b = sup.addSignal("errorUnrecoverableIPCFault(QService::UnrecoverableIPCError)");
  //d->ipcfailure = b.index();
  //_p->meta = sup.toMetaObject();
}

QiRemoteObject::~QiRemoteObject()
{
}

const QMetaObject* QiRemoteObject::metaObject() const {
  std::cout << "metaObject" << std::endl;
  return _p->meta;
};

int                QiRemoteObject::qt_metacall(QMetaObject::Call c, int id, void **a) {
  std::cout << "qt_metacall" << std::endl;
};

void              *QiRemoteObject::qt_metacast(const char* className) {
  std::cout << "qt_metacast" << std::endl;
}


//virtual void metaCall(const std::string &method, const std::string &sig, qi::DataStream &in, qi::DataStream &out) {
//  qi::Message msg;
//  msg.setId(200);
//  msg.setSource("ouame");
//  msg.setDestination(_dest);
//  msg.setPath(method);
//  msg.setData(in.str());
//  _ts->send(msg);
//  _ts->waitForId(msg.id());

//  qi::Message ret;
//  _ts->read(msg.id(), &ret);
//  out.str(ret.data());
//}

