/*
** Author(s):
**  - hcuche <hcuche@aldebaran-robotics.com>
**
** Copyright (C) 2012 hcuche
*/

// Leave this above the header gard, it fixes include ordering problems
# include <qimessaging/metavalue.hpp>

#ifndef _QIMESSAGING_BUFFER_HPP_
#define _QIMESSAGING_BUFFER_HPP_

# include <boost/shared_ptr.hpp>
# include <qimessaging/api.hpp>
# include <qimessaging/metavalue.hpp>

# include <vector>
# include <qi/types.hpp>
# include <qi/log.hpp>
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

    /// @return the size of this buffer, not counting subbuffers
    size_t size() const;

    /// @return the size of this buffer and all its sub-buffers.
    size_t totalSize() const;

    /** When a buffer is serialized into an other buffer, only the size is
     * written, and the other buffer gets appended into subBuffers() instead of
     * being copied. The first element of the pair is the offset at which
     * the buffer should have been inserted.
     *
     */
     std::vector<std::pair<uint32_t, Buffer> >&       subBuffers();
     const std::vector<std::pair<uint32_t, Buffer> >& subBuffers() const;

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
