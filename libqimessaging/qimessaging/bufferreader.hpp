/*
 *
 *  Copyright (C) 2012 Aldebaran Robotics
 */
#pragma once
#ifndef _QIMESSAGING_BUFFERREADER_HPP_
#define _QIMESSAGING_BUFFERREADER_HPP_

#include <qimessaging/buffer.hpp>

namespace qi {

  class BufferReader
  {
  public:
    BufferReader(const Buffer& buf);
    ~BufferReader();
    size_t read(void *data, size_t len);
    // equivalent to peek() && seek()
    void  *read(size_t size);
    size_t seek(long offset);
    void  *peek(size_t size) const;
    /// @return true if a subBuffer is available at current offset
    bool hasSubBuffer() const;
    Buffer getSubBuffer();
    size_t position() { return _cursor;}
  private:
    Buffer _buffer;
    size_t _cursor;
    size_t _subCursor; // position in subbuffers
  };
}

#endif
