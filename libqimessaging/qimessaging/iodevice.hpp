/*
** Author(s):
**  - hcuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 hcuche
*/

#pragma once
#ifndef _QIMESSAGING_IODEVICE_HPP_
#define _QIMESSAGING_IODEVICE_HPP_

namespace qi
{
  class QIMESSAGING_API IODevice
  {
  public:
    virtual ~IODevice() = 0;
    virtual int            write(const void *data, size_t size)   = 0;
    virtual int            prepend(const void *data, size_t size) = 0;

    virtual int            read(void *data, size_t size) = 0;
    virtual void          *peek(size_t size)             = 0;
    virtual int            drain(size_t size)            = 0;

    virtual unsigned int   size() const = 0;
  }; // !IODevice
} // !qi

#endif  // _QIMESSAGING_IODEVICE_HPP_
