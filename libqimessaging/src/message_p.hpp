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
# include <qi/types.hpp>

namespace qi
{
  class MessagePrivate
  {
  public:
    typedef struct
    {
      qi::uint32_t magic;
      qi::uint32_t id;
      qi::uint32_t size;
      qi::uint32_t type;
      qi::uint32_t service;
      qi::uint32_t path;
      qi::uint32_t function;
      qi::uint32_t reserved;
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
