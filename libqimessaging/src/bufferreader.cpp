/*
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include <cstring>

#include <qimessaging/bufferreader.hpp>
#include "src/buffer_p.hpp"
namespace qi {

  BufferReader::BufferReader(const Buffer& buf)
  : _buffer(buf)
  , _cursor(0)
  {
    ++_buffer._p->nReaders;
  }

  BufferReader::~BufferReader()
   {
     --_buffer._p->nReaders;
   }

  size_t BufferReader::seek(long offset)
  {
    if (_cursor + offset <= _buffer._p->used)
    {
      _cursor += offset;
      return _cursor;
    }
    else
    {
      return -1;
    }
  }

  void *BufferReader::peek(size_t size) const
  {
    if (_cursor + size <= _buffer._p->used)
      return _cursor + _buffer._p->data();
    else
      return 0;
  }

  void *BufferReader::read(size_t size)
  {
    void *p = 0;
    if ((p = peek(size)))
      seek(size);
    else
      return 0;

    return p;
  }

  size_t BufferReader::read(void *data, size_t size)
  {
    if (_buffer._p->used - _cursor < size)
    {
      size = _buffer._p->used - _cursor;
    }
    memcpy(data, _buffer._p->data() + _cursor, size);
    _cursor += size;

    return size;
  }
}
