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
#include "src/signature/pretty_print_signature_visitor.hpp"

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
  size_t len;
  const char *ss = stream.readString(len);
  QByteArray qba(ss, len);
  s.append(qba);
  return stream;
}

qi::DataStream& operator<<(qi::DataStream &stream, const QString &s)
{
  stream.writeString((const char *)s.toAscii().constData(), s.size());
  return stream;
}


void qi_MetaTypeStore(qi::DataStream &ds, int metatype, void *data)
{
  if (metatype ==  QMetaType::QString)
    ds << *(reinterpret_cast<QString *>(data));
  //if (type == "std::string")
  //  ds << reinterpret_cast<std::string>(data);
}

void qi_MetaTypeLoad(qi::DataStream &ds, int metatype, void *data)
{
  if (metatype ==  QMetaType::QString)
    ds >> *(reinterpret_cast<QString *>(data));
}


void qi_SignatureToMetaMethod(const std::string &signature, QString *returnSig, QString *funcSig)
{
  qi::PrettyPrintSignatureVisitor ppsv(signature.c_str(), qi::PrettyPrintSignatureVisitor::Qt);

  *returnSig = QString::fromStdString(ppsv.returnSignature());
  *funcSig = QString::fromStdString(ppsv.functionSignature());
}

