/*
** Author(s):
**  - hcuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 hcuche
*/

#pragma once
#ifndef _QIMESSAGING_BUFFER_HPP_
#define _QIMESSAGING_BUFFER_HPP_

# include <boost/shared_ptr.hpp>
# include <qimessaging/api.hpp>

# include <cstddef>

namespace qi
{
  class BufferPrivate;

  class QIMESSAGING_API Buffer
  {
  public:
    Buffer();
    Buffer(const Buffer& b);
    Buffer& operator = (const Buffer& b);
    int    write(const void *data, size_t size);

    size_t size() const;

    void   clear();

    /** Reserve "size" more bytes at the end and return a pointer to the data.
     * returned value is valid til the next non-const operation*/
    void*  reserve(size_t size);
    void*  data();
    const void* data() const;
    void*  read(size_t offset =0, size_t length=0) const;
    size_t read(void* buffer, size_t pos, size_t length) const;
    void   dump() const;
  private:
    // We need those friends for our reader-writer conflict detection system.
    friend class BufferReader;
    friend class ODataStream;
    boost::shared_ptr<BufferPrivate> _p;
  };

  //operator << and >> implemented in qi::DataStream to avoid header recursive inclusion

} // !qi

#endif  // _QIMESSAGING_BUFFER_HPP_
