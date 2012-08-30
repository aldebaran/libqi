/*
**
** Author(s):
**  - Cedric GESTES <gestes@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef   	_QIMESSAGING_MESSAGE_C_P_H_
# define   	_QIMESSAGING_MESSAGE_C_P_H_

#include <qimessaging/message.hpp>

typedef struct
{
  qi::ODataStream *os;
  qi::IDataStream *is;
  qi::Message     *msg;
  qi::Buffer      *buff;
} qi_message_data_t;

#endif

