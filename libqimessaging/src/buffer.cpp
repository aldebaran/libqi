/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/
#include <qimessaging/genericvalue.hpp>
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
    , available(sizeof(_data))
    , nReaders(0)
    , nWriters(0)
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

  Buffer::Buffer()
  {
  }

  Buffer::Buffer(const Buffer& b)
  {
    (*this) = b;
  }

  Buffer& Buffer::operator=(const Buffer& b)
  {
    if (!b._p)
      const_cast<Buffer&>(b)._p = boost::shared_ptr<BufferPrivate>(new BufferPrivate());
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

    qiLogDebug("qimessaging.buffer") << "Resizing buffer from " << available << " to " << neededSize;
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

  int Buffer::write(const void *data, size_t size)
  {
    if (!_p)
      _p = boost::shared_ptr<BufferPrivate>(new BufferPrivate());
    if (*_p->nReaders)
      qiLogWarning("qi.Buffer") << "write operation while readers are present";
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

  size_t Buffer::size() const
  {
    return _p?_p->used:0;
  }

  size_t Buffer::totalSize() const
  {
    if (!_p)
      return 0;
    size_t res = size();
    for (unsigned i=0; i< _p->_subBuffers.size(); ++i)
      res += _p->_subBuffers[i].second.totalSize();
    return res;
  }

  std::vector<std::pair<uint32_t, Buffer> >& Buffer::subBuffers()
  {
    return _p->_subBuffers;
  }

  const std::vector<std::pair<uint32_t, Buffer> > & Buffer::subBuffers() const
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
    if (!_p)
      _p = boost::shared_ptr<BufferPrivate>(new BufferPrivate());
    if (_p->used + size > _p->available)
      _p->resize(_p->used + size);

    void *p = _p->data() + _p->used;
    _p->used += size;

    return p;
  }

  void Buffer::clear()
  {
    if (_p)
    {
      _p->used = 0;
      _p->_subBuffers.clear();
    }
  }
  void* Buffer::data()
  {
    return _p ? _p->data() : 0;
  }

  const void* Buffer::data() const
  {
    return _p ? _p->data() : 0;
  }

  void *Buffer::read(size_t off, size_t length) const
  {
    if (!_p)
    {
      qiLogDebug("qimessaging.buffer") << "read on empty buffer";
      return 0;
    }
    if (off + length > _p->used)
    {
      qiLogDebug("qimessaging.buffer") << "Attempt to read " << off+length
       <<" on buffer of size " << _p->used;
      return 0;
    }
    return (char*)_p->data() + off;
  }

  size_t Buffer::read(void* buffer, size_t off, size_t length) const
  {
    if (!_p)
    {
      qiLogDebug("qimessaging.buffer") << "read on empty buffer";
      return -1;
    }
    if (off > _p->used)
    {
      qiLogDebug("qimessaging.buffer") << "Attempt to read " << off+length
      <<" on buffer of size " << _p->used;
      return -1;
    }
    size_t copy = std::min(length, _p->used - off);
    memcpy(buffer, (char*)_p->data()+off, copy);
    return copy;
  }

  void Buffer::dump() const
  {
    if (!_p)
    {
       qiLogDebug("qimessaging.buffer") << "dump on empty buffer";
      return;
    }
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

  std::string& Buffer::signature()
  {
    return _p->signature;
  }

  const std::string& Buffer::signature() const
  {
    return _p->signature;
  }

  class TypeBuffer: public TypeString
  {
  public:
    virtual std::pair<char*, size_t> get(void* storage) const
    {
      Buffer& b = *(Buffer*)const_cast<TypeBuffer*>(this)->ptrFromStorage(&storage);
      return std::make_pair((char*)b.data(), b.size());
    }
    virtual void set(void** storage, const char* ptr, size_t sz)
    {
      Buffer& b = *(Buffer*)ptrFromStorage(storage);
      memcpy(b.reserve(sz), ptr, sz);
    }
    virtual Buffer* asBuffer(void* storage)
    {
      Buffer& b = *(Buffer*)ptrFromStorage(&storage);
      return &b;
    }

    typedef DefaultTypeImplMethods<Buffer> Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };
} // !qi

QI_TYPE_REGISTER_CUSTOM(qi::Buffer, qi::TypeBuffer);
