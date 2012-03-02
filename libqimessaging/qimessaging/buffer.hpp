/*
** Author(s):
**  - hcuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 hcuche
*/

#pragma once
#ifndef _QIMESSAGING_BUFFER_HPP_
#define _QIMESSAGING_BUFFER_HPP_

# include <qimessaging/api.hpp>
# include <qimessaging/iodevice.hpp>

namespace qi
{
  class BufferPrivate;

  class QIMESSAGING_API Buffer: public IODevice
  {
  public:
    Buffer();
    ~Buffer();

    // Add data to the end of the buffer
    int    write(const void *data, size_t size);
    // Add data in front of the buffer
    int    prepend(const void *data, size_t size);
    // read size first data of the buffer
    // warning: linearize data if needed (copy)
    int    read(void *data, size_t size);
    void  *peek(size_t size);
    int    drain(size_t size);
    unsigned int size() const;

  public:
    BufferPrivate *_p;
  };


} // !qi

#endif  // _QIMESSAGING_BUFFER_HPP_
