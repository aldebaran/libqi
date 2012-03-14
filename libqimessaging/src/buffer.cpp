/*
** buffer.cpp
** Login : <hcuche@hcuche-de>
**
** Author(s):
**  - hcuche <hcuche@aldebaran-robotics.com>
*/

#include <qimessaging/buffer.hpp>

#include <cstdio>
#include <cstring>
#include <ctype.h>

#include "src/buffer_p.hpp"

#include <iostream>

namespace qi
{

  BufferPrivate::BufferPrivate()
  {
    size = 0;
    cursor = 0;
  }

  Buffer::Buffer()
    : _p(new BufferPrivate) // TODO: add allocation-on-write.
  {
  }

  size_t Buffer::write(const void *data, size_t size)
  {
    if (sizeof(_p->data) - _p->cursor < size)
    {
      return -1;
    }

    memcpy(_p->data + _p->size, data, size);
    _p->size += size;
    _p->cursor += _p->size;

    return size;
  }

  size_t Buffer::read(void *data, size_t size)
  {
    if (_p->size - _p->cursor < size)
    {
      size = _p->size - _p->cursor;
    }

    memcpy(data, _p->data + _p->cursor, size);
    _p->cursor += size;

    return size;
  }

  void *Buffer::read(size_t size)
  {
    void *p = 0;
    if ((p = peek(size)))
    {
      seek(size);
    }

    return p;
  }

  size_t Buffer::size() const
  {
    return _p->size;
  }

  void *Buffer::reserve(size_t size)
  {
    void *p = _p->data + _p->cursor;
    _p->size = size;

    return p;
  }

  size_t Buffer::seek(long offset)
  {
    if (_p->cursor + offset <= _p->size)
    {
      _p->cursor += offset;
      return _p->cursor;
    }
    else
    {
      return -1;
    }
  }

  void *Buffer::peek(size_t size) const
  {
    return _p->cursor + _p->data;
  }

  void *Buffer::data() const
  {
    return _p->data;
  }

  void Buffer::dump() const
  {
    unsigned int i = 0;

    while (i < _p->size)
    {
      printf("%02x ", _p->data[i]);
      i++;
      if (i % 8 == 0) printf(" ");
      if (i % 16 == 0)
      {
        for (unsigned int j = i - 16; j < i ; j++)
        {
          printf("%c", isgraph(_p->data[j]) ? _p->data[j] : '.');
        }
        printf("\n");
      }
    }

    while (i % 16 != 0)
    {
      printf("   ");
      if (i % 8 == 0) printf(" ");
      i++;
    }
    printf(" ");
    for (unsigned int j = i - 16; j < _p->size; j++)
    {
      printf("%c", isgraph(_p->data[j]) ? _p->data[j] : '.');
    }
    printf("\n");
  }

} // !qi
