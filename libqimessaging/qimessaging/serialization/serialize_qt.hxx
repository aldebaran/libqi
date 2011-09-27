#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011 Aldebaran Robotics
*/

#ifndef _QI_SERIALIZATION_SERIALIZE_QT_HXX_
#define _QI_SERIALIZATION_SERIALIZE_QT_HXX_

#include <QVector>
#include <QList>
#include <QMap>
#include <QString>
#include <qimessaging/serialization/message.hpp>

namespace qi {
  namespace serialization {

    template <>
    struct serialize<QString>  {
      static inline void write(Message &sd, const QString &val) {
        std::string hack = val.toStdString();
        sd.writeString(hack);
      }
      static inline void read(Message &sd, QString &val) {
        std::string hack;
        sd.readString(hack);
        val = QString::fromStdString(hack);
      }
    };

    QI_LIST_SERIALIZER(QVector);
    QI_LIST_SERIALIZER(QList);
    QI_MAP_SERIALIZER(QMap);

  }
}

#endif  // _QI_SERIALIZATION_SERIALIZE_HXX_
