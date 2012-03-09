/*
** Author(s):
**  - Herve CUCHE <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran-robotics
*/

#pragma once
#ifndef BUFFER_P_HPP_
# define BUFFER_P_HPP_

namespace qi
{
  class BufferPrivate
  {
  public:
    BufferPrivate();

    unsigned char   data[4096];
    size_t          size;
    size_t          cursor;
  };
}

#endif	    // !BUFFER_P_HPP_
