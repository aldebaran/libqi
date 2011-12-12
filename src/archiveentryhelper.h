/**
 * @author Jerome Vuarand
 * @author Victor Paleologue
 * Copyright (c) Aldebaran Robotics 2010, 2011 All Rights Reserved
 */
#pragma once
#ifndef ALPROJECT_ARCHIVEENTRYHELPER_H
#define ALPROJECT_ARCHIVEENTRYHELPER_H

#include <vector>
#include <archive.h>

namespace qi
{
  class ArchiveEntryHelper
  {
  private:
    static const size_t fBlocksize = 32 * 1024; // 32kB
    struct archive* fArchive;
    char* fBuffer;

  public:
    ArchiveEntryHelper(struct archive* archive);

    static int openForWrite(struct archive* a, void* pVoidPointer) { return static_cast<ArchiveEntryHelper*>(pVoidPointer)->xOpenForWrite(a); }
    static int openForRead(struct archive* a, void* pVoidPointer) { return static_cast<ArchiveEntryHelper*>(pVoidPointer)->xOpenForRead(a); }
    static __LA_SSIZE_T write(struct archive* a, void* pVoidPointer, const void* pBuffer, size_t pSize) { return static_cast<ArchiveEntryHelper*>(pVoidPointer)->xWrite(a, pBuffer, pSize); }
    static __LA_SSIZE_T read(struct archive* a, void* pVoidPointer, const void** pBufferPointer) { return static_cast<ArchiveEntryHelper*>(pVoidPointer)->xRead(a, pBufferPointer); }
    static int close(struct archive* a, void* pVoidPointer) { return static_cast<ArchiveEntryHelper*>(pVoidPointer)->xClose(a); }

  private:
    int xOpenForWrite(struct archive* a);
    int xOpenForRead(struct archive* a);
    __LA_SSIZE_T xWrite(struct archive* a, const void* pBuffer, size_t pSize);
    __LA_SSIZE_T xRead(struct archive* a, const void** pBufferPointer);
    int xClose(struct archive* a);
  };
}

#endif
