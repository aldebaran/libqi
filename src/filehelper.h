/**
 * @author Jerome Vuarand
 * @author Victor Paleologue
 * Copyright (c) Aldebaran Robotics 2010, 2011 All Rights Reserved
 */

#pragma once

#ifndef _LIBQI_QI_PROJECT_FILEHELPER_HPP
#define _LIBQI_QI_PROJECT_FILEHELPER_HPP

#include <boost/filesystem.hpp>
#include <archive.h>

namespace qi
{
  class FileHelper
  {
  private:
    static const size_t fBlocksize = 32*1024; // 32kB
    boost::filesystem::path fPath;
    FILE* fFile;
    std::vector<unsigned char> fBuffer;

  public:
    FileHelper(const boost::filesystem::path& path);

    static int openForWrite(struct archive* a, void* pVoidPointer)
    {
      return static_cast<FileHelper*>(pVoidPointer)->xOpenForWrite(a);
    }

    static int openForRead(struct archive* a, void* pVoidPointer) { return static_cast<FileHelper*>(pVoidPointer)->xOpenForRead(a); }
    static __LA_SSIZE_T write(struct archive* a, void* pVoidPointer, const void* pBuffer, size_t pSize) { return static_cast<FileHelper*>(pVoidPointer)->xWrite(a, pBuffer, pSize); }
    static __LA_SSIZE_T read(struct archive* a, void* pVoidPointer, const void** pBufferPointer) { return static_cast<FileHelper*>(pVoidPointer)->xRead(a, pBufferPointer); }
    static int close(struct archive* a, void* pVoidPointer) { return static_cast<FileHelper*>(pVoidPointer)->xClose(a); }

  private:
    int xOpenForWrite(struct archive* a);
    int xOpenForRead(struct archive* a);
    __LA_SSIZE_T xWrite(struct archive* a, const void* pBuffer, size_t pSize);
    __LA_SSIZE_T xRead(struct archive* a, const void** pBufferPointer);
    int xClose(struct archive* a);
  };
}

#endif // _LIBQI_QI_PROJECT_FILEHELPER_HPP
