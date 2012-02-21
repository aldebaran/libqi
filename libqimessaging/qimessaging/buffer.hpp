/*
** Author(s):
**  - hcuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 hcuche
*/

#ifndef BUFFER_HPP_
# define BUFFER_HPP_

# include <qimessaging/iodevice.hpp>

namespace qi
{
  class BufferPrivate;

  class Buffer: public IODevice
  {
  public:
    Buffer();
    ~Buffer();

    // Add data to the end of the buffer
    int            write(const void *data, size_t size);
    // Add data in front of the buffer
    int            prepend(const void *data, size_t size);
    // read size first data of the buffer
    // warning: linearize data if needed (copy)
    void* read(size_t size);

    int size() const;

    void *data();
    void setData(void *data);

  private:
    BufferPrivate *_p;
  };


} // !qi

#endif // !BUFFER_HPP_
