/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/buffer.hpp>
#include <qi/log.hpp>

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <iomanip>
#include <ctype.h>

#include <boost/pool/singleton_pool.hpp>

#include "buffer_p.hpp"

#include <iostream>

qiLogCategory("qi.Buffer");

namespace qi
{
  BufferPrivate::BufferPrivate()
    : _bigdata(0)
    , _cachedSubBufferTotalSize(0)
    , used(0)
    , available(sizeof(_data))
  {
  }

  BufferPrivate::~BufferPrivate()
  {
    if (_bigdata)
    {
      free(_bigdata);
      _bigdata = NULL;
    }
  }

  struct MyPoolTag { };
  typedef boost::singleton_pool<MyPoolTag, sizeof(BufferPrivate)> buffer_pool;

  void* BufferPrivate::operator new(size_t sz)
  {
    assert(sz <= sizeof(BufferPrivate));
    return buffer_pool::malloc();
  }

  void BufferPrivate::operator delete(void* ptr)
  {
    buffer_pool::free(ptr);
  }

  int BufferPrivate::indexOfSubBuffer(size_t offset) const
  {
    for(unsigned i = 0; i < _subBuffers.size(); ++i) {
      if (_subBuffers[i].first == offset) {
        return i;
      }
    }

    return -1;
  }

  Buffer::Buffer()
    : _p(boost::shared_ptr<BufferPrivate>(new BufferPrivate()))
  {
  }

  Buffer::Buffer(const Buffer& b)
  {
    (*this) = b;
  }

  Buffer& Buffer::operator=(const Buffer& b)
  {
    _p = b._p;
    return *this;
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

    qiLogDebug() << "Resizing buffer from " << available << " to " << neededSize;
    unsigned char *newBigdata;

    newBigdata = static_cast<unsigned char *>(realloc(_bigdata, neededSize));
    if (newBigdata == NULL)
      return false;
    if (!_bigdata && used > 0)
      ::memcpy(newBigdata, _data, used);
    available = neededSize;
    _bigdata = newBigdata; // Don't worry, realloc free previous buffer if needed
    return true;
  }

  bool Buffer::write(const void *data, size_t size)
  {
    if (_p->used + size > _p->available)
    {
      bool ret = _p->resize(_p->used + size);
      if (!ret) {
        qiLogVerbose() << "write(" << size << ") failed, buffer size is " << _p->available;
        return false;
      }
    }

    memcpy(_p->data() + _p->used, data, size);
    _p->used += size;

    return true;
  }

  size_t Buffer::addSubBuffer(const Buffer& buffer)
  {
    size_t subBufferSize = buffer.size();
    size_t actualUsed = _p->used;

    write((uint32_t*)&subBufferSize, sizeof(uint32_t));

    _p->_subBuffers.push_back(std::make_pair(actualUsed, buffer));
    _p->_cachedSubBufferTotalSize += buffer.totalSize();
    return actualUsed;
  }

  bool Buffer::hasSubBuffer(size_t offset) const
  {
    return (_p->indexOfSubBuffer(offset) != -1);
  }

  const Buffer& Buffer::subBuffer(size_t offset) const
  {
    int index = _p->indexOfSubBuffer(offset);

    if (index == -1)
      throw std::runtime_error("No sub-buffer at the specified offset.");

    return _p->_subBuffers[index].second;
  }

  size_t Buffer::size() const
  {
    return _p->used;
  }

  size_t Buffer::totalSize() const
  {
    return size() + _p->_cachedSubBufferTotalSize;
  }

  const std::vector<std::pair<size_t, Buffer> > & Buffer::subBuffers() const
  {
    return _p->_subBuffers;
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

  void Buffer::clear()
  {
    _p->used = 0;
    _p->_subBuffers.clear();
    _p->_cachedSubBufferTotalSize = 0;
  }

  void* Buffer::data()
  {
    return _p ? _p->data() : 0;
  }

  const void* Buffer::data() const
  {
    return _p ? _p->data() : 0;
  }

  const void *Buffer::read(size_t offset, size_t length) const
  {
    if (offset + length > _p->used)
    {
      qiLogDebug() << "Attempt to read " << offset+length
       <<" on buffer of size " << _p->used;
      return 0;
    }
    return (char*)_p->data() + offset;
  }

  size_t Buffer::read(void* buffer, size_t offset, size_t length) const
  {
    if (offset > _p->used)
    {
      qiLogDebug() << "Attempt to read " << offset+length
      <<" on buffer of size " << _p->used;
      return -1;
    }
    size_t copy = std::min(length, _p->used - offset);
    memcpy(buffer, (char*)_p->data()+offset, copy);
    return copy;
  }

  namespace details {
    void printBuffer(std::ostream& stream, const Buffer& buffer)
    {
      if (buffer.size() == 0) {
        qiLogDebug() << "dump on empty buffer";
        return;
      }

      std::ios_base::fmtflags flags = stream.flags();
      unsigned int i = 0;
      const unsigned char* data = (const unsigned char*)buffer.data();

      while (i < buffer.size()) {
        if (i % 16 == 0) stream << std::hex << std::setfill('0') << std::setw(8) << i << ": ";
        stream << std::setw(2) << (const unsigned int) data[i];
        i++;
        if (i % 2 == 0) stream << ' ';
        if (i % 16 == 0)
        {
          for (unsigned int j = i - 16; j < i ; j++)
          {
            char c = data[j];
            stream << (isgraph(c) ? c : '.');
          }
          stream << '\n';
        }
      }

      while (i % 16 != 0) {
        stream << "  ";
        if (i % 2 == 0) stream << ' ';
        i++;
      }
      stream << ' ';

      for (unsigned int j = i - 16; j < buffer.size(); j++) {
        char c = data[j];
        stream << (isgraph(c) ? c : '.');
      }

      stream.flags(flags);
    }
  }
} // !qi
