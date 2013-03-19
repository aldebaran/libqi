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

    //explicit BinaryDecoder(const qi::Buffer &buffer);
    explicit BinaryDecoder(qi::BufferReader *buffer);
    ~BinaryDecoder();


    enum Status {
      Status_Ok                     = 0,
      Status_ReadError              = 1,
      Status_ReadPastEnd            = 2,
    };

    void read(bool      &b);

    void read(char        &c);
    void read(signed char &c);
    void read(short       &s);
    void read(int         &i);
    void read(long        &l);
    void read(long long   &ll);

    void read(unsigned char  &uc);
    void read(unsigned short &us);
    void read(unsigned int   &ui);
    void read(unsigned long  &ul);
    void read(unsigned long long &ull);

    void read(float    &f);
    void read(double   &d);
    void read(std::string& i);

    void read(qi::GenericValuePtr &value);
    void read(qi::Buffer &buffer);

    template<typename T> void read(T& v);

    //read raw data
    size_t readRaw(void *data, size_t len);
    void* readRaw(size_t len);
    Status status() const;
    void setStatus(Status status);
    BufferReader& bufferReader();

  private:
    BinaryDecoderPrivate *_p;
  };

  class TransportSocket;
  typedef boost::shared_ptr<TransportSocket> TransportSocketPtr;

  namespace details {
    QIMESSAGING_API GenericValuePtr deserialize(qi::Type *type, BinaryDecoder& in, TransportSocketPtr context = TransportSocketPtr());
    /// Deserialize in place
    QIMESSAGING_API void deserialize(GenericValuePtr what, BinaryDecoder& in, TransportSocketPtr context = TransportSocketPtr());
  }

  template<typename T>
  void BinaryDecoder::read(T& v)
  {
    details::deserialize(GenericValueRef(v), *this);
  }
}

#endif // QIMESSAGING_BINARYDECODER_HPP
