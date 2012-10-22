#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_MESSAGE_P_HPP_
#define _SRC_MESSAGE_P_HPP_

# include <boost/cstdint.hpp>
# include <qi/buffer.hpp>
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
      qi::uint16_t version;
      qi::uint16_t type;
      qi::uint32_t service;
      qi::uint32_t object;
      qi::uint32_t action;
    } MessageHeader;

    MessagePrivate();
    ~MessagePrivate();

    inline void                complete() { header.size = buffer.totalSize(); }
    inline void               *getHeader() { return reinterpret_cast<void *>(&header); }

    Buffer        buffer;
    std::string   signature;
    MessageHeader header;

    static const unsigned int magic = 0x42adde42;
  };
}

#endif  // _SRC_MESSAGE_P_HPP_
