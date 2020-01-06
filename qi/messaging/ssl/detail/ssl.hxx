/*
**  Copyright (C) 2022 Aldebaran Robotics
**  See COPYING for the license
*/

#pragma once
#ifndef QI_MESSAGING_SSL_DETAIL_SSL_HXX
#define QI_MESSAGING_SSL_DETAIL_SSL_HXX

#include "../ssl.hpp"
#include <qi/numeric.hpp>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <stdexcept>

namespace qi
{
namespace ssl
{

namespace detail
{

template<typename O>
Pointer<O>::Pointer(pointer p, Owned owned)
  : p(p)
{
  if (owned == Owned::False)
    upRef();
}

template<typename O>
Pointer<O>::Pointer(const Pointer& o)
{
  o.upRef();
  p.reset(o.p.get());
}

template<typename O>
Pointer<O>& Pointer<O>::operator=(const Pointer& o)
{
  if (&o == this)
    return *this;
  o.upRef();
  p.reset(o.p.get());
  return *this;
}

template<typename O>
void Pointer<O>::upRef() const
{
  if (p == nullptr)
    return;
  ::ERR_clear_error();
  if (!Traits::upRef(p.get()))
  {
    throw Error::fromCurrentError("could not increment reference count of " +
                                  std::string(Traits::name()) + " object ");
  }
}

} // namespace detail

template<typename ContiIt>
BIO BIO::fromRangeImpl(ContiIt begin, ContiIt end)
{
  const auto data = const_cast<char*>(reinterpret_cast<const char*>(&*begin));
  const auto diff = std::distance(begin, end);
  // 'BIO_new_mem_buf' interprets a size of -1 as a note that the data is
  // null terminated (it uses `strlen` to compute the length). Therefore,
  // we can't expect the function to handle any negative size correctly.
  using ValueType = typename std::iterator_traits<ContiIt>::value_type;
  const uint64_t sizeu64 = diff < 0 ? 0 : diff * sizeof(ValueType);
  const auto size = qi::numericConvertBound<int>(sizeu64);
  const auto bio = ::BIO_new_mem_buf(data, size);
  if (!bio)
    throw Error::fromCurrentError("could not create BIO from range");
  return BIO(bio);
}

} // namespace ssl
} // namespace qi

#endif // QI_MESSAGING_SSL_DETAIL_SSL_HXX
