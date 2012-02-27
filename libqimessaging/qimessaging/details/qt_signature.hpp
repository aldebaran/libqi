/*
*  Author(s):
*  - Chris  Kilner <ckilner@aldebaran-robotics.com>
*  - Cedric Gestes <gestes@aldebaran-robotics.com>
*
*  Copyright (C) 2010 Aldebaran Robotics
*/


#pragma once
#ifndef _QIMESSAGING_DETAILS_QT_SIGNATURE_HPP_
#define _QIMESSAGING_DETAILS_QT_SIGNATURE_HPP_

# include <qimessaging/signature/detail/type_signature.hpp>
# include <string>
# include <QtCore/QtContainerFwd>

namespace qi {
  namespace detail {

    _QI_SIMPLE_SIGNATURE(QString, "s");
    _QI_LIST_SIGNATURE(QList);
    _QI_LIST_SIGNATURE(QVector);
    _QI_MAP_SIGNATURE(QMap);

  }
}

#endif  // _QIMESSAGING_DETAILS_QT_SIGNATURE_HPP_
