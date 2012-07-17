/*
**
** Author(s):
**  - Herve CUCHE <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran-robotics
*/

#ifndef MESSAGE_P_HPP_
# define MESSAGE_P_HPP_

# include <boost/cstdint.hpp>
# include <qimessaging/buffer.hpp>

namespace qi
{
  class MessagePrivate
  {
  public:
    typedef struct
    {
      uint32_t magic;
      uint32_t id;
      uint32_t size;
      uint32_t type;
      uint32_t service;
      uint32_t path;
      uint32_t function;
      uint32_t reserved;
    } MessageHeader;

    MessagePrivate();
    ~MessagePrivate();

    void                       complete();
    void                      *getHeader();

    Buffer        buffer;
    MessageHeader header;

    static const unsigned int magic = 0x42adde42;
  };
}

#endif	    // !MESSAGE_P_HPP_
