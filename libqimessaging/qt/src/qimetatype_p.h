/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	QIMETATYPE_P_H_
# define   	QIMETATYPE_P_H_

#include <QtCore/qstring.h>
#include <QtCore/qmetaobject.h>
#include <qimessaging/datastream.hpp>

QString qi_MetatypeToNetworkType(const int metatype);
QString qi_NetworkTypeToMetatype(const QString &metatype);

void qi_MetaTypeStore(qi::DataStream &ds, int metatype, void *data);
void qi_MetaTypeLoad(qi::DataStream &ds, int metatype, void *data);

void qi_SignatureToMetaMethod(const std::string &signature, QString *returnSig, QString *funcSig);

//void qi_MetaTypeStore(qi::DataStream &ds, const QString &type, void *data);



#endif 	    /* !QIMETATYPE_P_H_ */
