/**
 * @author Jerome Vuarand
 * Copyright (c) Aldebaran Robotics 2010, 2011 All Rights Reserved
 */
#pragma once
#ifndef ALPROJECT_MEMORYHELPER_H
#define ALPROJECT_MEMORYHELPER_H

#include <vector>
#include <deque>
#include <string>
#include <archive.h>

namespace qi
{
  class MemoryHelper
  {
  private:
    static const size_t fBlocksize = 32*1024; // 32kB
    std::vector<unsigned char> fBuffer;
    std::deque<unsigned char> fContent;

  public:
    MemoryHelper();
    MemoryHelper(const std::string& content);

    std::string content();

    static int openForWrite(struct archive* a, void* pVoidPointer) { return static_cast<MemoryHelper*>(pVoidPointer)->xOpenForWrite(a); }
    static int openForRead(struct archive* a, void* pVoidPointer) { return static_cast<MemoryHelper*>(pVoidPointer)->xOpenForRead(a); }
    static __LA_SSIZE_T write(struct archive* a, void* pVoidPointer, const void* pBuffer, size_t pSize) { return static_cast<MemoryHelper*>(pVoidPointer)->xWrite(a, pBuffer, pSize); }
    //static __LA_SSIZE_T read(struct archive* a, void* pVoidPointer, const void** pBufferPointer) { return static_cast<MemoryHelper*>(pVoidPointer)->xRead(a, pBufferPointer); }
    static int close(struct archive* a, void* pVoidPointer) { return static_cast<MemoryHelper*>(pVoidPointer)->xClose(a); }

  private:
    int xOpenForWrite(struct archive* a);
    int xOpenForRead(struct archive* a);
    __LA_SSIZE_T xWrite(struct archive* a, const void* pBuffer, size_t pSize);
    //__LA_SSIZE_T xRead(struct archive* a, const void** pBufferPointer);
    int xClose(struct archive* a);
  };
}

#endif
