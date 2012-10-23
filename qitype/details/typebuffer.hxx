#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef _QITYPE_DETAILS_TYPEBUFFER_HXX_
#define _QITYPE_DETAILS_TYPEBUFFER_HXX_

#include <qi/buffer.hpp>

namespace qi
{
  class TypeBufferImpl: public TypeRaw
  {
  public:
    virtual Buffer get(void *storage)
    {
      return *(Buffer*)ptrFromStorage(&storage);
    }
    virtual void set(void** storage, Buffer& value)
    {
      Buffer& b = *(Buffer*)ptrFromStorage(storage);
      b = value;
    }
    typedef DefaultTypeImplMethods<Buffer> Methods;
    _QI_BOUNCE_TYPE_METHODS(Methods);
  };

template<> class TypeImpl<Buffer>: public TypeBufferImpl {};
}

#endif
