#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010, 2011, 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_SERIALIZATION_SERIALIZE_QT_HXX_
#define _QIMESSAGING_SERIALIZATION_SERIALIZE_QT_HXX_

#include <QString>
#include <QtCore/QtContainerFwd>
#include <qimessaging/serialization/message.hpp>

namespace qi {
  namespace serialization {

    template <>
    struct serialize<QString>  {
      static inline void write(qi::DataStream &sd, const QString &val) {
        std::string hack = val.toStdString();
        sd.writeString(hack);
      }
      static inline void read(qi::DataStream &sd, QString &val) {
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

#endif  // _QIMESSAGING_SERIALIZATION_SERIALIZE_QT_HXX_
