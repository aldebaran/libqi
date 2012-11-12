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
#include <qimessaging/binaryencoder.hpp>
#include <qimessaging/binarydecoder.hpp>

typedef struct
{
  qi::BinaryEncoder *os;
  qi::BinaryDecoder *is;
  qi::Message     *msg;
  qi::Buffer      *buff;
} qi_message_data_t;

qi::BinaryEncoder &get_os(qi_message_data_t *m);
qi::BinaryDecoder &get_is(qi_message_data_t *m);
#endif

