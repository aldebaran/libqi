/*
**  Copyright (C) 2022 Aldebaran Robotics
**  See COPYING for the license
*/

#include <openssl/bio.h>
#include <openssl/err.h>
#include <qi/messaging/ssl/ssl.hpp>
#include <qi/log.hpp>

namespace qi
{
namespace ssl
{

const char* ErrorCategory::name() const noexcept
{
  return "qi.ssl";
}

std::string ErrorCategory::message(int value) const
{
  const auto s = ::ERR_reason_error_string(static_cast<unsigned long>(value));
  return s ? s : "unknown SSL error";
}

const ErrorCategory& errorCategory()
{
  static const ErrorCategory instance;
  return instance;
}

Error::Error(unsigned long err, const std::string& what)
  : std::system_error(static_cast<int>(err), errorCategory(), what)
{}

Error Error::fromCurrentError(const std::string& what)
{
  const auto err = ::ERR_peek_last_error();
  ::ERR_clear_error();
  return Error(err, what);
}

PemPasswordCallback
  PemPasswordCallback::fromPassphrase(const char* passphrase) noexcept
{
  return PemPasswordCallback(nullptr, const_cast<char*>(passphrase));
}

std::vector<char> BIO::memoryData() const
{
  char* buf = nullptr;
  ::ERR_clear_error();
  const auto len = ::BIO_get_mem_data(get(), &buf);
  if (len < 0) {
    throw Error::fromCurrentError("failure to get BIO memory data");
  }
  if (buf == nullptr)
    return {};
  return std::vector<char>(buf, buf + len);
}

std::vector<char> BIO::read(int size)
{
  if (size <= 0)
    return {};
  std::vector<char> buf;
  buf.resize(static_cast<std::size_t>(size));

  ::ERR_clear_error();
  const auto len = ::BIO_read(get(), buf.data(), size);
  if (len < 0)
    throw Error::fromCurrentError("failure to read from BIO");
  buf.resize(len);
  return buf;
}

boost::optional<Certificate>
  BIO::readPemX509(bool trusted, PemPasswordCallback passwordCb)
{
  ::ERR_clear_error();
  const auto readX509 = trusted ? ::PEM_read_bio_X509_AUX : ::PEM_read_bio_X509;
  const auto x509 = readX509(get(),
      nullptr, // X509 object used for initialization
      passwordCb.callback, passwordCb.userData
    );
  if (x509 == nullptr)
  {
    const auto err =  ::ERR_peek_last_error();
    if (ERR_GET_LIB(err) == ERR_LIB_PEM
        // No start line means there is no more certificate in the PEM.
        && ERR_GET_REASON(err) == PEM_R_NO_START_LINE)
    {
      ::ERR_clear_error();
      return {};
    }
    throw Error::fromCurrentError("could not read X509 from PEM");
  }
  return Certificate(x509);
}

boost::optional<PKey> BIO::readPemPrivateKey(PemPasswordCallback passwordCb)
{
  ::ERR_clear_error();
  auto key = ::PEM_read_bio_PrivateKey(get(),
      nullptr, // EVP_PKEY object used for initialization
      passwordCb.callback, passwordCb.userData
    );
  if (key == nullptr)
  {
    const auto err =  ::ERR_peek_error();
    if (
        // Sometimes, `PEM_read_bio_PrivateKey` returns null but without
        // an active error. In these cases, we consider that there was no
        // key in the file.
        err == 0
        || (ERR_GET_LIB(err) == ERR_LIB_PEM
          // No start line means there is no more key in the PEM.
          && ERR_GET_REASON(err) == PEM_R_NO_START_LINE))
    {
      ::ERR_clear_error();
      return {};
    }
    throw Error::fromCurrentError("could not read private key from PEM");
  }
  return PKey(key);
}

std::vector<Certificate> BIO::readPemX509Chain(PemPasswordCallback passwordCb)
{
  std::vector<Certificate> certs;
  bool aux = true;
  while (auto cert = readPemX509(aux, passwordCb))
  {
    certs.push_back(*std::move(cert));
    if (aux)
      aux = false;
  }
  return certs;
}

BIO BIO::fromFile(boost::filesystem::path path, const char* mode)
{
  const auto genericPath = path.generic_string();
  const auto bio = ::BIO_new_file(genericPath.c_str(), mode);
  if (!bio)
    throw Error::fromCurrentError("could not create BIO from file");
  return BIO(bio);
}

int PKey::eq(const PKey& o) const
{
  return ::EVP_PKEY_eq(get(), o.get());
}

void PKey::usePrivateIn(::SSL_CTX& ctx) const
{
  ::ERR_clear_error();

  // On success, SSL_CTX_use_PrivateKey returns 1. Otherwise check out the error stack to find out
  // the reason.
  if (::SSL_CTX_use_PrivateKey(&ctx, get()) != 1)
    throw Error::fromCurrentError("could not use private key in SSL context");
}

::X509_NAME* Certificate::subjectName() const noexcept
{
  return ::X509_get_subject_name(get());
}

int Certificate::cmp(const Certificate& o) const
{
  return ::X509_cmp(get(), o.get());
}

void Certificate::useIn(::SSL_CTX& ctx) const
{
  ::ERR_clear_error();

  // On success, SSL_CTX_use_certificate() returns 1. Otherwise check out the error stack to find
  // out the reason.
  if (::SSL_CTX_use_certificate(&ctx, get()) != 1)
    throw Error::fromCurrentError("could not use certificate in SSL context");
}

void Certificate::addToCurrentChain(::SSL_CTX& ctx) const
{
  ::ERR_clear_error();

  // Excerpt from the manual of SSL_CTX_add1_chain_cert(3ssl):
  //   All these functions are implemented as macros. Those containing a 1 increment the reference
  //   count of the supplied certificate or chain so it must be freed at some point after the operation.
  //   Those containing a 0 do not increment reference counts and the supplied certificate or chain
  //   MUST NOT be freed after the operation.
  //   All other functions return 1 for success and 0 for failure.
  if (::SSL_CTX_add1_chain_cert(&ctx, get()) == 0)
    throw Error::fromCurrentError("could not add certificate to SSL context extra chain certificates");
}

std::string to_string(const Certificate& cert)
{
  const auto bio = BIO(::BIO_new(::BIO_s_mem()));
  ::X509_print(bio.get(), cert.get());
  const auto certTextData = bio.memoryData();
  return std::string(certTextData.begin(), certTextData.end());
}

std::string to_string(const ::X509_NAME& name)
{
  const auto bio = BIO(::BIO_new(::BIO_s_mem()));
  ::X509_NAME_print(bio.get(), &name, 0);
  const auto certTextData = bio.memoryData();
  return std::string(certTextData.begin(), certTextData.end());
}

std::ostream& operator<<(std::ostream& os, const Certificate& cert)
{
  return os << to_string(cert);
}

std::ostream& operator<<(std::ostream& os, const ::X509_NAME& name)
{
  return os << to_string(name);
}

void CertificateStore::add(const Certificate& cert)
{
  ::ERR_clear_error();

  // Excerpt from the manual of X509_STORE_add_cert(3ssl):
  //   The added object's reference count is incremented by one, hence the caller retains ownership
  //   of the object and needs to free it when it is no longer needed.
  //   [...]
  //   X509_STORE_add_cert() [...] return 1 on success or 0 on failure.
  if (::X509_STORE_add_cert(get(), cert.get()) == 0)
    throw Error::fromCurrentError("could not add certificate to X509 store");
}

::X509_VERIFY_PARAM& CertificateStore::verifyParameters()
{
  ::ERR_clear_error();
  const auto param = ::X509_STORE_get0_param(get());
  if (!param)
    throw Error::fromCurrentError("could not get X509 store param");
  return *param;
}

void CertChainWithPrivateKey::useIn(::SSL_CTX& ctx) const
{
  if (certificateChain.empty() || !privateKey)
    throw std::runtime_error("missing certificate chain or certificate private key in SSL configuration");

  // The following algorithm is based on the implementation of the
  // `SSL_CTX_use_certificate_chain_file` function.
  auto certIt = certificateChain.begin();
  const auto certEnd = certificateChain.end();
  (certIt++)->useIn(ctx);
  if (certIt != certEnd)
  {
    ::ERR_clear_error();

    // SSL_CTX_clear_chain_certs returns 1 for success and 0 for failure.
    if (::SSL_CTX_clear_chain_certs(&ctx) == 0)
      throw Error::fromCurrentError("could not clear context certificates chain");

    for (; certIt != certEnd; ++certIt)
      certIt->addToCurrentChain(ctx);
  }

  privateKey.usePrivateIn(ctx);
}

} // namespace ssl
} // namespace qi
