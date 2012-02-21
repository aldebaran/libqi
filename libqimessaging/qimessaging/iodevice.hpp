/*
** Author(s):
**  - hcuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 hcuche
*/

#ifndef IODEVICE_HPP_
# define IODEVICE_HPP_

namespace qi
{
  class IODevice
  {
  public:
    IODevice() {}
    virtual ~IODevice() {}

  protected:
    virtual int            write(const void *data, size_t size)   = 0;
    virtual int            prepend(const void *data, size_t size) = 0;

    virtual void          *read(size_t size) = 0;

    virtual void          *data() = 0;
    virtual void           setData(void *data) = 0;

    virtual int            size() const = 0;


  }; // !IODevice
} // !qi

#endif // !IODEVICE_HPP_
