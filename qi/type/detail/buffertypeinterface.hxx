#pragma once
/*
**  Copyright (C) 2013 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAIL_TYPEBUFFER_HXX_
#define _QITYPE_DETAIL_TYPEBUFFER_HXX_

#include <qi/buffer.hpp>

namespace qi
{
  class TypeBufferImpl: public RawTypeInterface
  {
  public:
    std::pair<char*, size_t> get(void *storage) override
    {
      Buffer* b = (Buffer*)Methods::ptrFromStorage(&storage);

      // TODO: sub-buffers
      if (b->subBuffers().size() != 0)
        qiLogError("qitype.buffertypeinterface") << "buffer has sub-buffers, Python bytearrays might be incomplete";

      return std::make_pair(const_cast<char*>((const char*)b->data()), b->size());
    }
    void set(void** storage, const char* ptr, size_t sz) override
    {
      Buffer* b = (Buffer*)ptrFromStorage(storage);
      b->clear();
      b->write(ptr, sz);
    }
    using Methods = DefaultTypeImplMethods<Buffer, TypeByPointerPOD<Buffer> >;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

template<> class TypeImpl<Buffer>: public TypeBufferImpl {};
}

#endif  // _QITYPE_DETAIL_TYPEBUFFER_HXX_
