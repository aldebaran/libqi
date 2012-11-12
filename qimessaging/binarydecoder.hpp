#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QIMESSAGING_BINARYDECODER_HPP
#define QIMESSAGING_BINARYDECODER_HPP

#include <qi/buffer.hpp>
#include <qi/bufferreader.hpp>

#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>

#include <qimessaging/api.hpp>

namespace qi {
  class GenericValuePtr;
  class BinaryDecoderPrivate;

  /** This class provides data serialization, using
   * a qi::Buffer as a backend.
   *
   *
   */
  class QIMESSAGING_API BinaryDecoder {
  public:

    explicit BinaryDecoder(const qi::Buffer &buffer);
    ~BinaryDecoder();


    enum Status {
      Status_Ok                     = 0,
      Status_ReadError              = 1,
      Status_ReadPastEnd            = 2,
    };

    BinaryDecoder& operator>>(bool      &b);

    BinaryDecoder& operator>>(char        &c);
    BinaryDecoder& operator>>(signed char &c);
    BinaryDecoder& operator>>(short       &s);
    BinaryDecoder& operator>>(int         &i);
    BinaryDecoder& operator>>(long        &l);
    BinaryDecoder& operator>>(long long   &ll);

    BinaryDecoder& operator>>(unsigned char  &uc);
    BinaryDecoder& operator>>(unsigned short &us);
    BinaryDecoder& operator>>(unsigned int   &ui);
    BinaryDecoder& operator>>(unsigned long  &ul);
    BinaryDecoder& operator>>(unsigned long long &ull);

    BinaryDecoder& operator>>(float    &f);
    BinaryDecoder& operator>>(double   &d);
    BinaryDecoder& operator>>(std::string& i);

    BinaryDecoder& operator>>(qi::GenericValuePtr &value);
    BinaryDecoder& operator>>(qi::Buffer &buffer);

    //read raw data
    size_t read(void *data, size_t len);
    void* read(size_t len);
    Status status() const;
    void setStatus(Status status);
    BufferReader& bufferReader();

  private:
    BinaryDecoderPrivate *_p;
  };

  template<typename T>
  BinaryDecoder& operator>>(BinaryDecoder& in, T& v);

  namespace details {
    QIMESSAGING_API GenericValuePtr deserialize(qi::Type *type, BinaryDecoder& in);
  }

  template<typename T>
  BinaryDecoder& operator>>(BinaryDecoder& in, T& v)
  {
    Type* type = typeOf<T>();
    GenericValuePtr value = qi::details::deserialize(type, in);;
    T* ptr = (T*)type->ptrFromStorage(&value.value);
    v = *ptr;
    return in;
  }
}

#endif // QIMESSAGING_BINARYDECODER_HPP
