/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _QIMESSAGING_ERROR_P_H_
# define _QIMESSAGING_ERROR_P_H_

# include <qic/api.h>

# ifdef __cplusplus
extern "C"
{
# endif // !__cplusplus

bool qi_c_set_error(const std::string& error);

# ifdef __cplusplus
}
# endif // !__cplusplus

#endif // !_QIMESSAGING_ERROR_P_H_
