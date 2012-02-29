/*
** Author(s):
**  - Herve CUCHE <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran-robotics
*/

#pragma once
#ifndef BUFFER_P_HPP_
# define BUFFER_P_HPP_

# include <event2/buffer.h>

namespace qi
{
  class BufferPrivate
  {
  public:
    BufferPrivate();
    ~BufferPrivate();

    struct evbuffer *data();
    void setData(struct evbuffer *data);

    void dump();

  public:
    struct evbuffer *_bufev;
  };
}

#endif	    // !BUFFER_P_HPP_
