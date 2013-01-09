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
    void* operator new(size_t);
    void operator delete(void*);
    unsigned char * data();
    bool            resize(size_t size = 0x100000);
    int             indexOfSubBuffer(size_t offset) const;

  public:
    unsigned char*  _bigdata;
    unsigned char   _data[STATIC_BLOCK];
    size_t          _cachedSubBufferTotalSize;

  public:
    size_t          used; // size used
    size_t          available; // total size of buffer

    std::vector<std::pair<size_t, Buffer> > _subBuffers;
  };
}

#endif  // _SRC_BUFFER_P_HPP_
