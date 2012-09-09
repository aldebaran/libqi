/*
** Author(s):
**  - Laurent LEC <llec@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#pragma once
#ifndef QIMESSAGING_QT_API_H_
# define QIMESSAGING_QT_API_H_

# include <qi/macro.hpp>

# ifdef qimessaging_qt_EXPORTS
#  define QIMESSAGING_QT_API QI_EXPORT_API
# else
#  define QIMESSAGING_QT_API QI_IMPORT_API
# endif

#endif /* ifndef QIMESSAGING_QT_API_H_ */
