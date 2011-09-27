#pragma once
/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#ifndef _QI_SIGNATURE_DETAIL_TYPE_SIGNATURE_HPP_
#define _QI_SIGNATURE_DETAIL_TYPE_SIGNATURE_HPP_

# include <qimessaging/signature/detail/type_signature.hpp>
# include <string>
# include <QString>
# include <QVector>
# include <QMap>

namespace qi {
  namespace detail {

    _QI_SIMPLE_SIGNATURE(QString, "s");
    _QI_VECTOR_SIGNATURE(QList)
    _QI_VECTOR_SIGNATURE(QVector)
    _QI_MAP_SIGNATURE(QMap)

  }
}

#endif  // _QI_SIGNATURE_DETAIL_TYPE_SIGNATURE_HPP_
