#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _SRC_BUFFER_P_HPP_
#define _SRC_BUFFER_P_HPP_

#define STATIC_BLOCK 768
#define BLOCK   4096

#include <vector>
#include <qi/atomic.hpp>
#include <qi/types.hpp>

namespace qi
{
  class BufferPrivate
  {
  public:
    BufferPrivate();
    ~BufferPrivate();
    unsigned char * data();
    bool            resize(size_t size = 1048576);

  public:
    unsigned char*  _bigdata;
    unsigned char   _data[STATIC_BLOCK];

  public:
    size_t          used; // size used
    size_t          available; // total size of buffer

    // Used to serialize the Buffer signature
    std::string signature;

    std::vector<std::pair<uint32_t, Buffer> > _subBuffers;
  };
}

#endif  // _SRC_BUFFER_P_HPP_
