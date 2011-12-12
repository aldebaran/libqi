/**
 * @author Jerome Vuarand
 * @author Victor Paleologue
 * Copyright (c) Aldebaran Robotics 2010, 2011 All Rights Reserved
 */
#include "archiveentryhelper.h"

namespace qi
{

ArchiveEntryHelper::ArchiveEntryHelper(struct archive* archive)
  : fArchive(archive),
    fBuffer(NULL)
{
}

int ArchiveEntryHelper::xOpenForWrite(struct archive* a)
{
  if (archive_write_get_bytes_in_last_block(a) == -1)
    archive_write_set_bytes_in_last_block(a, 1);
  return ARCHIVE_OK;
}

int ArchiveEntryHelper::xOpenForRead(struct archive* a)
{
  fBuffer = new char[fBlocksize];
  return ARCHIVE_OK;
}

__LA_SSIZE_T ArchiveEntryHelper::xWrite(struct archive* a, const void* pBuffer, size_t pSize)
{
  return archive_write_data(fArchive, pBuffer, pSize);
}

__LA_SSIZE_T ArchiveEntryHelper::xRead(struct archive* a, const void** pBufferPointer)
{
  __LA_SSIZE_T result = archive_read_data(fArchive, fBuffer, fBlocksize);
  *pBufferPointer = fBuffer;
  return result;
}

int ArchiveEntryHelper::xClose(struct archive* a)
{
  if (fBuffer != NULL)
  {
    delete[] fBuffer;
    fBuffer = NULL;
  }
  return 0;
}

}
