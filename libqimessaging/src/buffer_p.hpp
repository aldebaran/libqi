/*
** Author(s):
**  - Herve CUCHE <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran-robotics
*/

#pragma once
#ifndef BUFFER_P_HPP_
# define BUFFER_P_HPP_

#define STATIC_BLOCK 768
#define BLOCK   4096

#include <qi/atomic.hpp>

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
    // Keep count of linked DataStream to try to report dangerous
    // parallel read and writes.
    qi::atomic<long> nReaders;
    qi::atomic<long> nWriters;
  };
}

#endif	    // !BUFFER_P_HPP_
