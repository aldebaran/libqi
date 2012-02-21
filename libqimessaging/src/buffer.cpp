/*
** buffer.cpp
** Login : <hcuche@hcuche-de>
**
** Author(s):
**  - hcuche <hcuche@aldebaran-robotics.com>
*/

#include <event2/buffer.h>
#include <qimessaging/buffer.hpp>


namespace qi
{
class BufferPrivate
{
public:
  BufferPrivate()
    : _bufev(evbuffer_new())
  {
  }

  ~BufferPrivate()
  {
  }

  struct evbuffer *_bufev;
};

  Buffer::Buffer()
    : _p(new BufferPrivate())
  {
  }

  Buffer::~Buffer()
  {
  }

  int Buffer::write(const void *data, size_t size)
  {
    return evbuffer_add(_p->_bufev, data, size);
  }

  int Buffer::prepend(const void *data, size_t size)
  {
    return evbuffer_prepend(_p->_bufev, data, size);
  }

  int Buffer::read(void *data, size_t size)
  {
    return evbuffer_remove(_p->_bufev, data, size);
  }

  void *Buffer::peek(size_t size)
  {
    return evbuffer_pullup(_p->_bufev, size);
  }

  int Buffer::drain(size_t size)
  {
    return evbuffer_drain(_p->_bufev, size);
  }

  size_t Buffer::size() const
  {
    return evbuffer_get_length(_p->_bufev);
  }

  void *Buffer::data()
  {
    return (void*)_p->_bufev;
  }

  void Buffer::setData(void *data)
  {
    _p->_bufev = reinterpret_cast<struct evbuffer *>(data);
  }

} // !qi
