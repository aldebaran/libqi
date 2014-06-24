#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QI_BUFFERREADER_HPP_
# define _QI_BUFFERREADER_HPP_

# include <qi/buffer.hpp>

namespace qi {

  /**
   * \brief Class to read const buffer.
   * \includename{qi/bufferreader.hpp}
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
}

#endif  // _QI_BUFFERREADER_HPP_
