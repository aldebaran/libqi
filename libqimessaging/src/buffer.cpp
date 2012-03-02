/*
** buffer.cpp
** Login : <hcuche@hcuche-de>
**
** Author(s):
**  - hcuche <hcuche@aldebaran-robotics.com>
*/

#include <event2/buffer.h>
#include <qimessaging/buffer.hpp>

#include <cstdio>
#include <cstdlib>

#include "src/buffer_p.hpp"

namespace qi
{

  BufferPrivate::BufferPrivate()
    : _bufev(evbuffer_new())
  {
  }

  BufferPrivate::~BufferPrivate()
  {
  }


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

  unsigned int Buffer::size() const
  {
    return evbuffer_get_length(_p->_bufev);
  }

  struct evbuffer *BufferPrivate::data()
  {
    return _bufev;
  }

  void BufferPrivate::setData(struct evbuffer *data)
  {
    _bufev = data;
  }

  void BufferPrivate::dump()
  {
    size_t size = evbuffer_get_length(_bufev);
    unsigned char *buf = (unsigned char*)malloc(size * sizeof(unsigned char));
    evbuffer_copyout(_bufev, buf, size);

    size_t i = 0;
    while (i < size)
    {
      printf("%02x ", *buf);
      ++buf;
      ++i;
    }
    printf("\n");
    fflush(stdout);
  }

} // !qi
