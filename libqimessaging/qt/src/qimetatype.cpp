/*
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <iostream>
#include <QtCore/qmetatype.h>
#include <QtCore/qbytearray.h>
#include "qimetatype_p.h"
#include "qmetaobjectbuilder_p.h"
#include <qimessaging/signature.hpp>

QString qi_MetatypeToNetworkType(const int metatype)
{
  if (metatype == QMetaType::QString)
    return "s";
  return "FAIL";
}

//QString qi_NetworkTypeToMetatype(const QString &metatype, const QMap<QString, QString> &typemap)
QString qi_NetworkTypeToMetatype(const QString &nettype)
{
  if (nettype == "s")
    return "QString";
  return "FAIL";
}


// string
qi::DataStream& operator>>(qi::DataStream &stream, QString &s)
{
  int len;
  stream >> len;
  QByteArray qbb(len, '0');
  stream.read(qbb.data(), len);
  s.append(qbb);
  return stream;
}

qi::DataStream& operator<<(qi::DataStream &stream, const QString &s)
{
  stream.writeString((const char *)s.toAscii().constData(), s.size());
  return stream;
}


bool qi_MetaTypeStore(qi::DataStream &stream, int metatype, void *data)
{
  if (!data || !QMetaType::isRegistered(metatype))
    return false;

  switch(metatype) {
    case QMetaType::Void:
    case QMetaType::VoidStar:
      return false;

    case QMetaType::Char:
      // force a char to be signed
      stream << *static_cast<const signed char *>(data);
      break;
    case QMetaType::UChar:
      stream << *static_cast<const uchar *>(data);
      break;
    case QMetaType::Bool:
      stream << qint8(*static_cast<const bool *>(data));
      break;

    case QMetaType::Int:
      stream << *static_cast<const int *>(data);
      break;
//    case QMetaType::UInt:
//      stream << *static_cast<const unsigned int *>(data);
//      break;

    case QMetaType::Short:
      stream << *static_cast<const short *>(data);
      break;
//    case QMetaType::UShort:
//      stream << *static_cast<const unsigned short *>(data);
//      break;

//    case QMetaType::Long:
//      stream << qlonglong(*static_cast<const long *>(data));
//      break;
//    case QMetaType::ULong:
//      stream << qulonglong(*static_cast<const ulong *>(data));
//      break;

//    case QMetaType::LongLong:
//      stream << *static_cast<const qlonglong *>(data);
//      break;
//    case QMetaType::ULongLong:
//      stream << *static_cast<const qulonglong *>(data);
//      break;

    case QMetaType::Float:
      stream << *static_cast<const float *>(data);
      break;
    case QMetaType::Double:
      stream << *static_cast<const double *>(data);
      break;

//    case QMetaType::QByteArray:
//      stream << *static_cast<const QByteArray*>(data);
//      break;
    case QMetaType::QString:
      stream << *static_cast<const QString*>(data);
      break;
//    case QMetaType::QStringList:
//      stream << *static_cast<const QStringList*>(data);
//      break;
    default:
      return false;
  }
  return true;
}

bool qi_MetaTypeLoad(qi::DataStream &stream, int metatype, void *data)
{
  if (!data || !QMetaType::isRegistered(metatype))
      return false;

  switch(metatype) {
    case QMetaType::Void:
    case QMetaType::VoidStar:
      return false;

    case QMetaType::Bool: {
      char b;
      stream >> b;
      *static_cast<bool *>(data) = b;
      break;
    }

//    case QMetaType::Char:
//      stream >> *static_cast<signed char *>(data);
//      break;
//    case QMetaType::UChar:
//      stream >> *static_cast<unsigned char *>(data);
//      break;

//    case QMetaType::Short:
//      stream >> *static_cast<short *>(data);
//      break;
//    case QMetaType::UShort:
//      stream >> *static_cast<ushort *>(data);
//      break;

    case QMetaType::Int:
      stream >> *static_cast<int *>(data);
      break;
//    case QMetaType::UInt:
//      stream >> *static_cast<uint *>(data);
//      break;

//    case QMetaType::Long: {
//      qlonglong l;
//      stream >> l;
//      *static_cast<long *>(data) = long(l);
//      break; }
//    case QMetaType::ULong: {
//      qulonglong ul;
//      stream >> ul;
//      *static_cast<ulong *>(data) = ulong(ul);
//      break; }
//    case QMetaType::LongLong:
//      stream >> *static_cast<qlonglong *>(data);
//      break;
//    case QMetaType::ULongLong:
//      stream >> *static_cast<qulonglong *>(data);
//      break;

    case QMetaType::Float:
      stream >> *static_cast<float *>(data);
      break;
    case QMetaType::Double:
      stream >> *static_cast<double *>(data);
      break;

//    case QMetaType::QByteArray:
//      stream >> *static_cast< NS(QByteArray)*>(data);
//      break;
    case QMetaType::QString:
      stream >> *static_cast< QString*>(data);
      break;
    default:
      return false;
  }
  return true;


}


void qi_SignatureToMetaMethod(const std::string &signature, QString *returnSig, QString *funcSig)
{
  std::vector<std::string> sigs = qi::signatureSplit(signature);

  qi::Signature rets(sigs[0]);
  qi::Signature funs(sigs[2]);
  *returnSig = QString::fromStdString(rets.toQtSignature(false));
  *funcSig = QString::fromStdString(sigs[1]) + QString::fromStdString(funs.toQtSignature(true));
}

