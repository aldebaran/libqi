/**
 * @author Jerome Vuarand
 * @author Victor Paleologue
 * Copyright (c) Aldebaran Robotics 2010, 2011 All Rights Reserved
 */

#include "filehelper.h"
#include <qi/os.hpp>
#include <string.h> // for strerror

namespace qi
{

FileHelper::FileHelper(const boost::filesystem::path& path)
  : fPath(path)
  , fFile(0)
{
}

int FileHelper::xOpenForWrite(struct archive* a)
{
  fFile = qi::os::fopen(fPath.c_str(), "wb");
  if (fFile)
  {
    if (archive_write_get_bytes_in_last_block(a) == -1)
      archive_write_set_bytes_in_last_block(a, 1);
    return ARCHIVE_OK;
  }
  else
  {
    printf("Reading archive: %s", strerror(errno));
    archive_set_error(a, errno, strerror(errno));
    return ARCHIVE_FATAL;
  }
}

int FileHelper::xOpenForRead(struct archive* a)
{
  fFile = qi::os::fopen(fPath.c_str(), "rb");
  if (fFile)
  {
    fBuffer.resize(fBlocksize);
    return ARCHIVE_OK;
  }
  else
  {
    archive_set_error(a, errno, strerror(errno));
    return ARCHIVE_FATAL;
  }
}

__LA_SSIZE_T FileHelper::xWrite(struct archive* a, const void* pBuffer, size_t pSize)
{
  return fwrite(pBuffer, 1, pSize, fFile);
}

__LA_SSIZE_T FileHelper::xRead(struct archive* a, const void** pBufferPointer)
{
  __LA_SSIZE_T result = fread(&fBuffer[0], 1, fBuffer.size(), fFile);
  *pBufferPointer = &fBuffer[0];
  return result;
}

int FileHelper::xClose(struct archive* a)
{
  if (fFile)
  {
    fBuffer.resize(0);
    fclose(fFile);
    fFile = NULL;
  }
  return 0;
}

}
