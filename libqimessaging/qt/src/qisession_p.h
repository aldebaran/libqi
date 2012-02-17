/*
** Author(s):
**  - Laurent Lec  <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	QISESSION_P_H_
# define   	QISESSION_P_H_

#include <QtCore/qobject.h>
#include <qimessaging/session.hpp>

class QiSessionPrivate {
public:
  qi::Session session;
};

#endif 	    /* !QISESSION_P_H_ */
