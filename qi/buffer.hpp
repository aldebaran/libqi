#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_BUFFER_HPP_
# define _QI_BUFFER_HPP_

# include <qi/api.hpp>
# include <qi/types.hpp>
# include <boost/shared_ptr.hpp>
# include <vector>
# include <cstddef>

#ifdef _MSC_VER
#  pragma warning( push )
#  pragma warning( disable: 4251 )
#endif

namespace qi
{
  class BufferPrivate;

  /**
   * \brief Class to store buffer.
   * \includename{qi/buffer.hpp}
   *
   * \verbatim
   * This class can store buffer and sub-buffers.
   * Here is a representation of internal management of sub-buffers.
   *
   * .. graphviz::
   *
   *      digraph g {
   *          graph [ rankdir = "LR" ];
   *          node [ fontsize = "16", shape = "ellipse" ];
   *
   *          subgraph cluster_buffer {
   *              mainbuffer;
   *              label = "Main buffer";
   *          }
   *
   *          subgraph cluster_subbuffer1 {
   *              subbuffer1;
   *              label = "Sub-buffer 1";
   *          }
   *
   *          subgraph cluster_subbuffer2 {
   *              subbuffer2;
   *              label = "Sub-buffer 2";
   *          }
   *
   *          "mainbuffer" [
   *              label = "...| ...| ...| ...| ...| ...|<f0> uint32_t subBufferSize| ...| ...|<f1> uint32_t subBufferSize| ...| ...| ..."
   *              shape = "record"
   *          ];
   *          "subbuffer1" [
   *              label = "<f0> ...| ...|...| ...|  ...|  ...| ...| ...| ...| ..."
   *              shape = "record"
   *          ];
   *          "subbuffer2" [
   *              label = "<f0> ...| ...|...| ...|  ...|  ...| ...| ...| ...| ..."
   *              shape = "record"
   *          ];
   *          "mainbuffer":f0-> "subbuffer1":f0[
   *              id = 0
   *          ];
   *          "mainbuffer":f1-> "subbuffer2":f0[
   *              id = 0
   *          ];
   *      }
   *
   * \endverbatim
   */
  class QI_API Buffer
  {
  public:
    /// \brief Default constructor.
    Buffer();

    /**
     * \brief Copy constructor.
     * \param buffer The buffer to copy.
     *
     * As data are store as a shared pointer, the different copy
     * of the same buffer all handle the same data.
     */
    Buffer(const Buffer& buffer);
    /**
     * \brief Assignment operator.
     * As data are store as a shared pointer, the different copy
     * of the same buffer all handle the same data.
     * \param buffer The buffer to copy.
     */
    Buffer& operator = (const Buffer& buffer);

    /**
     * \brief Move constructor.
     * The moved-from buffer's state is like a default constructed Buffer.
     * \param buffer The buffer to move from.
     */
    Buffer(Buffer&& buffer);

    /**
     * \brief Move assignment operator.
     * The moved-from buffer's state is like a default constructed Buffer.
     * \param buffer The buffer to move from.
     */
    Buffer& operator = (Buffer&& buffer);

    /**
     * \brief Write data in the buffer.
     * \param data The data to write
     * \param size The size of the data to write
     * \return true if operation succeeded, false otherwise.
     */
    bool write(const void *data, size_t size);

    /**
     * \brief Add a sub-buffer to the main buffer.
     * This function add a uint32_t for the size of sub-buffers in main buffer
     * and add the buffer to the list of sub-buffers.
     * \param buffer The buffer to have as sub-buffer.
     * \return return te offset at which sub-buffer have been added.
     */
    size_t addSubBuffer(const Buffer& buffer);
    /**
     * \brief Check if there is a sub-buffer at given offset.
     * \param offset The offset to look at the presence of sub-buffer.
     * \return true if there is a sub-buffer, false otherwise.
     */
    bool   hasSubBuffer(size_t offset) const;
    /**
     * \brief Return the sub-buffer at given offset.
     * If there is no sub-buffer throw a std::runtime_error.
     * \param offset The offset to look for sub-buffer.
     * \return the sub-buffer.
     */
    const  Buffer& subBuffer(size_t offset) const;


    /**
     * \brief Return the content size of this buffer not counting sub-buffers.
     * \return the size.
     * \see totalSize
     */
    size_t size() const;

    /**
     * \brief Return the content size of this buffer and of all its sub-buffers.
     * \return the size.
     * \see size
     */
    size_t totalSize() const;

    /**
     * \brief Return a vector of sub-buffers of the current buffer.
     * \return a vector of pairs. The first value of the pair is the offset of the
     * sub-buffer into the master buffer. The second value is the sub-buffer itself.
     */
    const std::vector<std::pair<size_t, Buffer> >& subBuffers() const;

    /**
     * \brief Reserve bytes at the end of current buffer.
     * \param size number of new bytes to reserve at the end of buffer.
     * \return a pointer to the data.
     * \warning The return value is valid until the next non-const operation.
     */
    void* reserve(size_t size);
    /**
     * \brief Erase content of buffer and remove sub-buffers whithout clearing them.
     */
    void  clear();


    /**
     * \brief Return a pointer to the raw data storage of this buffer.
     * \return the pointer to the data.
     */
    void* data();
    /**
     * \brief Return a const pointer to the raw data in this buffer.
     * \return the pointer to the data.
     */
    const void* data() const;

    /**
     * \brief Read some data from the buffer.
     * \param offset offset at which reading begin in the buffer.
     * \param length length of the data to read.
     * \return 0 if the buffer is empty or if we try to read data after the end
     * of the buffer.\n
     * Otherwise return a pointer to the data. Only \a length bytes can be read in
     * the returned buffer.
     */
    const void* read(size_t offset = 0, size_t length = 0) const;

    /**
     * \brief Read some data in the buffer and store it in a new pre-allocated buffer.
     * \warning the given buffer must be freed.
     * \param buffer the pre-allocated buffer to store data.
     * \param offset Offset in the current buffer to start copy.
     * \param length Length of the data to be copied.
     * \return -1 if there is no data in buffer or if \a offset is bigger than
     * total data in buffer.\n
     * Otherwise return the total length of copied data.
     */
    size_t read(void* buffer, size_t offset = 0, size_t length = 0) const;

  private:
    friend class BufferReader;
    // CS4251
    boost::shared_ptr<BufferPrivate> _p;
  };

  /**
   * \brief Class to read const buffer.
   * \includename{qi/buffer.hpp}
   * This class is intendeed to read buffer.
   * It store an internal data cursor and an internal sub-buffer index.
   * All offset are relative to the current position.
   */
  class QI_API BufferReader
  {
  public:
    /**
     * \brief Constructor.
     * \param buf The buffer to copy.
     */
    explicit BufferReader(const Buffer& buf);
    /// \brief Default destructor.
    ~BufferReader();

    /**
     * \brief read and store data from the buffer.
     * \param data A pre-allocated pointer to store read data.
     * \param length Size of the object pointed by \a data or size to read.
     * \return size of really read and stored data.
     */
    size_t read(void *data, size_t length);


    /**
     * \brief read data from buffer.
     * \param offset Number of bytes to read.
     * \return a pointer to data at the given
     */
    void  *read(size_t offset);
    /**
     * \brief Move forward the buffer cursor by the given offset.
     * \param offset Value for move forward the cursor.
     * \return Return true if succeed, false otherwise.
     */
    bool   seek(size_t offset);
    /**
     * \brief Check if we can read from the actual position toward \a offset bytes.
     * \warning This function doesn't move the internal pointer.
     * \param offset The relative offset.
     * \return The pointer if it succeed. If actual position +
     * \a offset exceed size of buffer return 0.
     */
    void  *peek(size_t offset) const;


    /**
     * \brief Check if there is sub-buffer at the actual position.
     * \return true if there is sub-buffer, false otherwise.
     */
    bool hasSubBuffer() const;
    /**
     * \brief return the sub-buffer at the actual position.
     * If there is no sub-buffer at actual position throw a std::runtime-error.
     * \return Return the sub-buffer if any.
     */
    const Buffer& subBuffer();
    /**
     * \brief Return the actual position in the buffer.
     * \return The current offset.
     */
    size_t position() const;

  private:
    Buffer _buffer;
    size_t _cursor;
    size_t _subCursor; // position in sub-buffers
  };

  namespace detail {
    QI_API void printBuffer(std::ostream& stream, const Buffer& buffer);
  }

}

#ifdef _MSC_VER
#  pragma warning( pop )
#endif

#endif  // _QI_BUFFER_HPP_
