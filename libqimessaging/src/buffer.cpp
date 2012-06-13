/*
** buffer.cpp
** Login : <hcuche@hcuche-de>
**
** Author(s):
**  - hcuche <hcuche@aldebaran-robotics.com>
*/

#include <qimessaging/buffer.hpp>
#include <qi/log.hpp>

#include <cstdio>
#include <cstring>
#include <ctype.h>

#include "src/buffer_p.hpp"

#include <iostream>

namespace qi
{

  BufferPrivate::BufferPrivate()
    : _bigdata(0)
    , used(0)
    , cursor(0)
    , available(sizeof(_data))
  {
  }

  BufferPrivate::~BufferPrivate()
  {
    if (_bigdata)
      delete _bigdata;
  }

  Buffer::Buffer()
    : _p(new BufferPrivate) // TODO: add allocation-on-write.
  {
  }

  unsigned char* BufferPrivate::data()
  {
    if (_bigdata)
      return (_bigdata);

    return (_data);
  }

  bool BufferPrivate::resize(size_t neededSize)
  {
    neededSize += BLOCK; // Should be enough in most cases;

    qiLogDebug("qimessaging.buffer") << "Resizing buffer from " << available << " to " << neededSize;
    if (_bigdata) // If we're already on heap, realloc
    {
      unsigned char *newBigdata;

      if ((newBigdata = static_cast<unsigned char *>(realloc(_bigdata, neededSize))) == NULL)
        return (false);
      available = neededSize;
      _bigdata = newBigdata; // Don't worry, realloc free previous buffer if needed
      return (true);
    }
    else
    {
      _bigdata = new unsigned char[neededSize];
      available = neededSize;

      if (used)
        ::memcpy(_bigdata, _data, used);

      return (true);
    }
  }

  int Buffer::write(const void *data, size_t size)
  {

    if (_p->used + size > _p->available)
    {
      bool ret = _p->resize(_p->used + size);
      if (!ret) {
        qiLogVerbose("qi.Buffer") << "write(" << size << ") failed, buffer size is " << _p->available;
        return -1;
      }
    }

    memcpy(_p->data() + _p->used, data, size);
    _p->used += size;

    return size;
  }

  int Buffer::read(void *data, size_t size)
  {
    if (_p->used - _p->cursor < size)
    {
      size = _p->used - _p->cursor;
    }

    memcpy(data, _p->data() + _p->cursor, size);
    _p->cursor += size;

    return size;
  }

  void *Buffer::read(size_t size)
  {
    void *p = 0;
    if ((p = peek(size)))
      seek(size);
    else
      return 0;

    return p;
  }

  size_t Buffer::size() const
  {
    return _p->used;
  }

  /*
  ** We need to allocate memory as soon as this function is called
  ** Returned memory MUST be coherent with the buffer used (either on stack/heap)
  ** As we can't know if the buffer will be used on heap later, we've to resize on heap
  */
  void *Buffer::reserve(size_t size)
  {
    if (_p->used + size > _p->available)
      _p->resize(_p->used + size);

    void *p = _p->data() + _p->used;
    _p->used += size;

    return p;
  }

  size_t Buffer::seek(long offset)
  {
    if (_p->cursor + offset <= _p->used)
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
    if (_p->cursor + size <= _p->used)
      return _p->cursor + _p->data();
    else
      return 0;
  }

  void *Buffer::data() const
  {
    return _p->data();
  }

  void Buffer::dump() const
  {
    unsigned int i = 0;

    while (i < _p->used)
    {
      printf("%02x ", _p->data()[i]);
      i++;
      if (i % 8 == 0) printf(" ");
      if (i % 16 == 0)
      {
        for (unsigned int j = i - 16; j < i ; j++)
        {
          printf("%c", isgraph(_p->data()[j]) ? _p->data()[j] : '.');
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
    for (unsigned int j = i - 16; j < _p->used; j++)
    {
      printf("%c", isgraph(_p->data()[j]) ? _p->data()[j] : '.');
    }
    printf("\n");
  }

} // !qi
