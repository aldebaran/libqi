/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include "qiremoteobject_p.h"
#include "qmetaobjectbuilder_p.h"
#include "qimetatype_p.h"

#include <qimessaging/datastream.hpp>
#include <qimessaging/genericobject.hpp>
#include <QtCore/qdebug.h>
#include <qimessaging/signature.hpp>
#include <qimessaging/transportsocket.hpp>

class QiRemoteObjectPrivate {
public:
  qi::TransportSocketPtr socket;
  std::string          dest;
  QMetaObject         *meta;
  unsigned int         serviceId;
};


QiRemoteObject::QiRemoteObject(qi::TransportSocketPtr ts, const std::string &dest, unsigned int serviceId, const qi::MetaObject &metaobject)
  : _p (new QiRemoteObjectPrivate)
{
  QMetaObjectBuilder ob;
  ob.setClassName(dest.c_str());

  qi::MetaObject::MethodMap::const_iterator it;
  for (it = metaobject.methodMap().begin(); it != metaobject.methodMap().end(); ++it) {
    const qi::MetaMethod &mm = it->second;
    QString retSig;
    QString funSig;
    qi_SignatureToMetaMethod(mm.signature(), &retSig, &funSig);
    //qDebug() << "Ret: " << retSig;
    //qDebug() << "Fun: " << funSig;
    QMetaMethodBuilder mmb = ob.addMethod(funSig.toUtf8(), retSig.toUtf8());
    mmb.setTag(mm.signature().c_str());
  }

  _p->socket = ts;
  _p->dest = dest;
  _p->serviceId = serviceId;
  _p->meta = ob.toMetaObject();
}

QiRemoteObject::~QiRemoteObject()
{
}

const QMetaObject* QiRemoteObject::metaObject() const {
  return _p->meta;
};

int                QiRemoteObject::qt_metacall(QMetaObject::Call c, int id, void **a) {
  qi::Message    msg;
  qi::Buffer     buf;
  qi::Message    retmsg;
  qi::ODataStream args(buf);

  msg.setBuffer(buf);

  if (c != QMetaObject::InvokeMetaMethod)
    return id;

  QMetaMethod method             = _p->meta->method(id);
  const int returnType           = QMetaType::type(method.typeName());
  const QList<QByteArray> pTypes = method.parameterTypes();
  const int pTypesCount          = pTypes.count();

  for (int i = 0; i < pTypesCount; i++) {
    const QByteArray &t = pTypes[i];
    const int ttype     = QMetaType::type(t);
    qi_MetaTypeStore(args, ttype, a[i+1]);
  }


  QString methodname(method.signature());
  methodname.truncate(methodname.indexOf('('));

  msg.setType(qi::Message::Type_Call);
  msg.setService(_p->serviceId);
  msg.setObject(qi::Message::GenericObject_Main);
  msg.setFunction(id - _p->meta->methodOffset());
  _p->socket->send(msg);

  //_p->socket->waitForId(msg.id());
  //_p->socket->read(msg.id(), &retmsg);

  qi::IDataStream retv(retmsg.buffer());

  qi_MetaTypeLoad(retv, returnType, a[0]);

  return 0;
};

void              *QiRemoteObject::qt_metacast(const char* className) {
  std::cout << "qt_metacast:" << className << std::endl;
  return 0;
}

