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
  QMetaMethodBuilder qmb;
  //qmb.
  mob.addMethod("reply(const std::string &)", "std::string");
  _p->meta = mob.toMetaObject();
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

//  if (c == QMetaObject::InvokeMetaMethod) {

//    const int mcount = d->meta->methodCount() - d->meta->methodOffset();
//    const int metaIndex = id + d->meta->methodOffset();

//    QMetaMethod method = d->meta->method(metaIndex);

//    const int returnType = QMetaType::type(method.typeName());

//    //process arguments
//    const QList<QByteArray> pTypes = method.parameterTypes();
//    const int pTypesCount = pTypes.count();
//    QVariantList args ;
//    if (pTypesCount > 10) {
//      qWarning() << "Cannot call" << method.signature() << ". More than 10 parameter.";
//      return id;
//    }
//    for (int i=0; i < pTypesCount; i++) {
//      const QByteArray& t = pTypes[i];

//      int variantType = QVariant::nameToType(t);
//      if (variantType == QVariant::UserType)
//        variantType = QMetaType::type(t);

//      if (t == "QVariant") {  //ignore whether QVariant is declared as metatype
//        args << *reinterpret_cast<const QVariant(*)>(a[i+1]);
//      } else if ( variantType == 0 ){
//        qWarning("%s: argument %s has unknown type. Use qRegisterMetaType to register it.",
//                 method.signature(), t.data());
//        return id;
//      } else {
//        args << QVariant(variantType, a[i+1]);
//      }
//    }

//    //QVariant looks the same as Void type. we need to distinguish them
//    if (returnType == QMetaType::Void && strcmp(method.typeName(),"QVariant") ) {
//      d->endPoint->invokeRemote(d->localToRemote[metaIndex], args, returnType);
//    } else {
//      //TODO: ugly but works
//      //add +1 if we have a variant return type to avoid triggering of void
//      //code path
//      //invokeRemote() parameter list needs review
//      QVariant result = d->endPoint->invokeRemote(d->localToRemote[metaIndex], args,
//                                                  returnType==0 ? returnType+1: returnType);
//      if (result.type() != QVariant::Invalid){
//        if (returnType != 0 && strcmp(method.typeName(),"QVariant")) {
//          QByteArray buffer;
//          QDataStream stream(&buffer, QIODevice::ReadWrite);
//          QMetaType::save(stream, returnType, result.constData());
//          stream.device()->seek(0);
//          QMetaType::load(stream, returnType, a[0]);
//        } else {
//          if (a[0]) *reinterpret_cast< QVariant*>(a[0]) = result;
//        }
//      }
//    }
//    id-=mcount;
//  }
//  return id;
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

