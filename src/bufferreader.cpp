/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <cstring>
#include <stdexcept>

#include <qi/bufferreader.hpp>
#include "buffer_p.hpp"
namespace qi {

  BufferReader::BufferReader(const Buffer& buffer)
  : _cursor(0)
  , _subCursor(0)
  , _buffer(buffer)
  {
  }

  BufferReader::~BufferReader()
  {
  }

  bool BufferReader::seek(size_t offset)
  {
    if (_cursor + offset <= _buffer.size())
    {
      _cursor += offset;
      return true;
    }
    else
    {
      return false;
    }
  }

  void *BufferReader::peek(size_t offset) const
  {
    if (_cursor + offset <= _buffer.size())
      return _cursor + (unsigned char*)_buffer.data();
    else
      return 0;
  }

  void *BufferReader::read(size_t offset)
  {
    void *p = 0;
    if ((p = peek(offset)))
      seek(offset);
    else
      return 0;

    return p;
  }

  size_t BufferReader::read(void *data, size_t size)
  {
    if (_buffer.size() - _cursor < size)
    {
      size = _buffer.size() - _cursor;
    }
    memcpy(data, (unsigned char*)(_buffer.data()) + _cursor, size);
    _cursor += size;

    return size;
  }

  bool BufferReader::hasSubBuffer() const
  {
    if (_buffer.subBuffers().size() <= _subCursor)
      return false;
    if (_buffer.subBuffers()[_subCursor].first == _cursor)
      return true;
    return false;
  }

  const Buffer& BufferReader::subBuffer()
  {
    if (!hasSubBuffer())
      throw std::runtime_error("No sub-buffer at actual offset.");

    _cursor += sizeof(uint32_t);

    return _buffer.subBuffers()[_subCursor++].second;
  }

  size_t BufferReader::position() const
  {
    return _cursor;
  }
}
