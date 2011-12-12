/**
 * @author Jerome Vuarand
 * @author Victor Paleologue
 * Copyright (c) Aldebaran Robotics 2010, 2011 All Rights Reserved
 */
#include "memoryhelper.h"

#include <algorithm> // for copy
#include <iterator> // for back_inserter

using std::string;

namespace qi
{

MemoryHelper::MemoryHelper()
{
}

MemoryHelper::MemoryHelper(const string& content)
  : fContent(content.begin(), content.end())
{
}

string MemoryHelper::content()
{
  return string(fContent.begin(), fContent.end());
}

int MemoryHelper::xOpenForWrite(struct archive* a)
{
  if (archive_write_get_bytes_in_last_block(a) == -1)
    archive_write_set_bytes_in_last_block(a, 1);
  fContent.clear();
  return ARCHIVE_OK;
}

int MemoryHelper::xOpenForRead(struct archive* a)
{
  fContent.clear();
  return ARCHIVE_OK;
}

__LA_SSIZE_T MemoryHelper::xWrite(struct archive* a, const void* pBuffer, size_t pSize)
{
  const unsigned char* buffer = static_cast<const unsigned char*>(pBuffer);
  copy(buffer, buffer+pSize, back_inserter(fContent));
  return pSize;
}

/*__LA_SSIZE_T MemoryHelper::xRead(struct archive* a, const void** pBufferPointer)
{
  throw std::exception("not implemented");
}*/

int MemoryHelper::xClose(struct archive* a)
{
  return 0;
}

}
