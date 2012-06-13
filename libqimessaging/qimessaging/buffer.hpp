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

    int    write(const void *data, size_t size);
    int    read(void *data, size_t size);

    // equivalent to peek() && seek()
    //returned value is valid til the next non-const operation
    void  *read(size_t size);
    size_t size() const;

    //returned value is valid til the next non-const operation
    void  *reserve(size_t size);
    size_t seek(long offset);
    void  *peek(size_t size) const;

    void  *data() const;
    void   dump() const;

  private:
    boost::shared_ptr<BufferPrivate> _p;
  };


} // !qi

#endif  // _QIMESSAGING_BUFFER_HPP_
