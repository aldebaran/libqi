#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QIMESSAGING_FILE_HPP_
#define _QIMESSAGING_FILE_HPP_

#include <qimessaging/api.hpp>
#include <qimessaging/binaryencoder.hpp>
#include <qimessaging/binarydecoder.hpp>
#include <string>

namespace qi
{
  class PrivateFile;

  enum QIMESSAGING_API Flag
  {
    Flag_Invalid   = 0,
    Flag_Write     = 1 << 0,
    Flag_Read      = 1 << 1,
    Flag_Create    = 1 << 2,
    Flag_Truncate  = 1 << 3,
  };

  class QIMESSAGING_API File
  {
  public:
    File();
    ~File();

    /**
     *  \brief Open a qi::file in memory.
     *  \param flags Flags for the qi::file.
     *  \return True on success, false otherwise.
     */
    bool open(int flags);

    /**
     *  \brief Associate a qi::file with a file.
     *  \param flags Flags for the qi::file.
     *  \param path Path to the file.
     *  \return True on success, false otherwise.
     */
    bool open(const std::string &path,
              int                flags);

    /**
     *  \brief Save the qi::file into dst path.
     *  \param dst Path to the file.
     *  \return True on success, false otherwise.
     */
    bool save(const std::string &dst);

    /**
     *  \brief Get the path to the file.
     *  \return Path to the file, or std::string() if the qi::file is in memory.
     */
    const std::string &path() const;

    /**
     *  \brief Get the file name.
     *  \return Name of the file, or std::string() if the qi::file is in memory.
     */
    const std::string &fileName() const;

    /**
     *  \brief Get flags of the qi::file.
     *  \return File's flags.
     */
    int flags() const;

  public:
    PrivateFile *p;
  };

  QIMESSAGING_API qi::BinaryEncoder &operator<<(qi::BinaryEncoder &stream,
                                             const qi::File &sfile);
  QIMESSAGING_API qi::BinaryDecoder &operator>>(qi::BinaryDecoder &stream,
                                             qi::File &sfile);
}

#endif  // _QIMESSAGING_FILE_HPP_
