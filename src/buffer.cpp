/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#include <qi/assert.hpp>
#include <qi/buffer.hpp>
#include <qi/log.hpp>

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <iomanip>
#include <ctype.h>
#include <algorithm>

#include <boost/make_shared.hpp>
#include <boost/utility/compare_pointees.hpp>

#include "buffer_p.hpp"


qiLogCategory("qi.Buffer");

namespace qi
{
  BufferPrivate::BufferPrivate() = default;

  BufferPrivate::~BufferPrivate()
  {
    if (_bigdata)
    {
      free(_bigdata);
      _bigdata = NULL;
    }
  }

  BufferPrivate::BufferPrivate(const BufferPrivate& b)
    : _bigdata(nullptr)
    , _cachedSubBufferTotalSize(b._cachedSubBufferTotalSize)
    , used(b.used)
    , available(b.available)
    , _subBuffers(b._subBuffers)
  {
    if (b._bigdata)
    {
      _bigdata = static_cast<unsigned char*>(malloc(b.used));
      ::memcpy(_bigdata, b._bigdata, b.used);
    }
    else
    {
      ::memcpy(_data, b._data, b.used);
    }
  }

  BufferPrivate& BufferPrivate::operator=(const BufferPrivate& b)
  {
    if (&b == this) return *this;
    _cachedSubBufferTotalSize = b._cachedSubBufferTotalSize;
    used = b.used;
    available = b.available;
    _subBuffers = b._subBuffers;
    if (_bigdata)
    {
      free(_bigdata);
      _bigdata = NULL;
    }
    if (b._bigdata)
    {
      _bigdata = static_cast<unsigned char*>(malloc(b.used));
      ::memcpy(_bigdata, b._bigdata, b.used);
    }
    else
    {
      ::memcpy(_data, b._data, b.used);
    }
    return *this;
  }

  boost::optional<size_t> BufferPrivate::indexOfSubBuffer(size_t offset) const
  {
    for(unsigned i = 0; i < _subBuffers.size(); ++i) {
      if (_subBuffers[i].first == offset) {
        return i;
      }
    }

    return {};
  }

  namespace
  {
    bool dataEqual(const unsigned char* aData, std::size_t aSize,
                   const unsigned char* bData, std::size_t bSize)
    {
      if (aSize != bSize) return false;
      const bool aIsNull = (aData == nullptr);
      const bool bIsNull = (bData == nullptr);
      if (aIsNull != bIsNull) return false;
      if (aIsNull) return true;  // Both are null.
      // Neither a nor b are null. We also know that their size are equal.
      return std::equal(aData, aData + aSize, bData);
    }
  }

  bool BufferPrivate::operator==(const BufferPrivate& o) const
  {
    // The "available" member is ignored as it does not affect the behavior of the object.
    // Idem for _cachedSubBufferTotalSize which is a cached data.
    return dataEqual(data(), used, o.data(), o.used) && _subBuffers == o._subBuffers;
  }

  unsigned char* BufferPrivate::data()
  {
    return _bigdata ? _bigdata : _data;
  }

  const unsigned char* BufferPrivate::data() const
  {
    return const_cast<BufferPrivate*>(this)->data();
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

  Buffer::Buffer()
    : _p(boost::make_shared<BufferPrivate>())
  {
  }

  Buffer::Buffer(const Buffer& b)
    : _p(boost::make_shared<BufferPrivate>(*b._p))
  {
  }

  Buffer& Buffer::operator=(const Buffer& b)
  {
    _p = boost::make_shared<BufferPrivate>(*b._p);
    return *this;
  }

  Buffer::Buffer(Buffer&& b)
    : _p(std::move(b._p))
  {
    // The default state of a qi::Buffer contains a valid BufferPrivate pointer.
    b._p = boost::make_shared<BufferPrivate>();
  }

  Buffer& Buffer::operator=(Buffer&& b)
  {
    _p = std::move(b._p);
    b._p = boost::make_shared<BufferPrivate>();
    return *this;
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

    write((size_type*)&subBufferSize, sizeof(size_type));

    _p->_subBuffers.push_back(std::make_pair(actualUsed, buffer));
    _p->_cachedSubBufferTotalSize += buffer.totalSize();
    return actualUsed;
  }

  bool Buffer::hasSubBuffer(size_t offset) const
  {
    return _p->indexOfSubBuffer(offset) ? true : false;
  }

  const Buffer& Buffer::subBuffer(size_t offset) const
  {
    if (const auto index = _p->indexOfSubBuffer(offset))
    {
      return _p->_subBuffers[*index].second;
    }
    else
    {
      throw std::runtime_error("No sub-buffer at the specified offset.");
    }
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
  **
  ** Returns a pointer to the first available byte in the (potentially
  ** reallocated) buffer memory, or nullptr in case of error.
  */
  void *Buffer::reserve(size_t size)
  {
    if (_p->used + size > _p->available)
    {
      bool success = _p->resize(_p->used + size);
      if (!success) {
        qiLogVerbose() << "reserve(" << size << ") failed, buffer size is " << _p->available;
        return nullptr;
      }
    }

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
      return  nullptr;
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

  bool Buffer::operator==(const Buffer& b) const
  {
    return boost::equal_pointees(_p, b._p);
  }

  namespace detail {
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
        stream << std::setw(2) << static_cast<unsigned int>(data[i]);
        i++;
        if (i % 2 == 0) stream << ' ';
        if (i % 16 == 0)
        {
          for (unsigned int j = i - 16; j < i ; j++)
          {
            const auto c = data[j];
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
        const auto c = data[j];
        stream << (isgraph(c) ? c : '.');
      }

      stream.flags(flags);
    }
  }
} // !qi
