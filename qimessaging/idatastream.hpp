#pragma once
/*
**  Copyright (C) 2012 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef QIMESSAGING_IDATASTREAM_HPP
#define QIMESSAGING_IDATASTREAM_HPP

#include <qi/buffer.hpp>
#include <qi/bufferreader.hpp>

#include <qitype/type.hpp>
#include <qitype/genericvalue.hpp>
#include <qitype/genericobject.hpp>

#include <qimessaging/api.hpp>

namespace qi {
  class GenericValuePtr;
  class IDataStreamPrivate;

  /** This class provides data serialization, using
   * a qi::Buffer as a backend.
   *
   *
   */
  class QIMESSAGING_API IDataStream {
  public:

    explicit IDataStream(const qi::Buffer &buffer);
    ~IDataStream();


    enum Status {
      Status_Ok                     = 0,
      Status_ReadError              = 1,
      Status_ReadPastEnd            = 2,
    };

    IDataStream& operator>>(bool      &b);

    IDataStream& operator>>(char        &c);
    IDataStream& operator>>(signed char &c);
    IDataStream& operator>>(short       &s);
    IDataStream& operator>>(int         &i);
    IDataStream& operator>>(long        &l);
    IDataStream& operator>>(long long   &ll);

    IDataStream& operator>>(unsigned char  &uc);
    IDataStream& operator>>(unsigned short &us);
    IDataStream& operator>>(unsigned int   &ui);
    IDataStream& operator>>(unsigned long  &ul);
    IDataStream& operator>>(unsigned long long &ull);

    IDataStream& operator>>(float    &f);
    IDataStream& operator>>(double   &d);
    IDataStream& operator>>(std::string& i);

    IDataStream& operator>>(qi::GenericValuePtr &value);
    IDataStream& operator>>(qi::Buffer &buffer);

    //read raw data
    size_t read(void *data, size_t len);
    void* read(size_t len);
    Status status() const;
    void setStatus(Status status);
    BufferReader& bufferReader();

  private:
    IDataStreamPrivate *_p;
  };

  template<typename T>
  IDataStream& operator>>(IDataStream& in, T& v);

  namespace details {
    QIMESSAGING_API GenericValuePtr deserialize(qi::Type *type, IDataStream& in);
  }

  template<typename T>
  IDataStream& operator>>(IDataStream& in, T& v)
  {
    Type* type = typeOf<T>();
    GenericValuePtr value = qi::details::deserialize(type, in);;
    T* ptr = (T*)type->ptrFromStorage(&value.value);
    v = *ptr;
    return in;
  }
}

#endif // QIMESSAGING_IDATASTREAM_HPP
