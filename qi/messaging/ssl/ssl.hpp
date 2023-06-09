/*
**  Copyright (C) 2022 Aldebaran Robotics
**  See COPYING for the license
*/

#pragma once
#ifndef QI_MESSAGING_SSL_SSL_HPP
#define QI_MESSAGING_SSL_SSL_HPP

#include <ka/macroregular.hpp>
#include <ka/utility.hpp>
#include <ka/typetraits.hpp>
#include <qi/api.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <memory>
#include <vector>

namespace qi
{

/// Namespace defining types and functions around the OpenSSL library.
///
/// It only forms a subset of all the features available in the OpenSSL library, by design.
/// The goal is to provide the main types and functions that are useful for the
/// qi library (and maybe user code), and nothing more. Features that are
/// not provided by this namespace must be implemented in user code.
///
/// Users of the qi library will mainly be exposed to the `ssl::ClientConfig` and
/// `ssl::ServerConfig` types which may be passed where the qi library offers
/// customization points for the SSL configuration of its internal messaging layers.
///
/// In most cases, the construction of these types will only consist of setting a
/// certificate and a private key, and optionally fill the certificate trust store
/// to verify peers certificates.
///
/// Example: reading a X509 certificate and a private key from two PEM files.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// auto cert = qi::ssl::Certificate::fromPemFile("/path/to/my/cert.pem");
/// auto key = qi::ssl::PKey::privateFromPemFile("/path/to/my/key.key");
/// // cert and key are optionals, reflecting the presence or not of a certificate
/// // and a key in the files.
/// if (!cert || !key)
///   throw std::runtime_error("could not find a certificate and a key in the files");
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
/// Example: reading a X509 certificate chain from a PEM file.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// auto certChain = qi::ssl::Certificate::chainFromPemFile("/path/to/my/chain.pem");
/// // certChain is a vector of certificates.
/// if (certChain.empty())
///   throw std::runtime_error("could not read any certificate from "
///                            "the certificate chain file");
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Example: reading a private key from a buffer.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// std::string keyData =
///   "-----BEGIN PRIVATE KEY-----\n"
///   [...]
///   "-----END PRIVATE KEY-----\n";
/// auto key = qi::ssl::PKey::privateFromPemRange(keyData);
/// if (!key)
///   throw std::runtime_error("could not read key from data");
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
///
/// Example: constructing a client configuration with a certificate, its private
///          key and a root CA certificate as trust anchor.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// qi::ssl::Certificate cert = ...;
/// qi::ssl::PKey        key = ...;
/// qi::ssl::Certificate rootCA = ...;
///
/// qi::ssl::CertChainWithPrivateKey certWithPKey;
/// // certificateChain is a vector of certificates representing a chain of
/// // certificates, but it may also contains only one certificate (which must
/// // be the end entity).
/// certWithPKey.certificateChain = { cert };
/// certWithPKey.privateKey = key;
/// qi::ssl::ClientConfig clientCfg;
/// clientCfg.certWithPrivKey = certWithPKey;
/// clientCfg.trustStore = { rootCA }; // or `trustStore.push_back(rootCA)`.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
namespace ssl
{

struct PKey;
struct Certificate;
struct Error;

/// An enumeration that reflects the ownership of an OpenSSL object.
/// If an object is owned, we can store it in pointer directly and it will be released once the
/// pointer is destroyed. If it is not owned, we must increase the reference count before storing it
/// inside the pointer, so that the pointer may release it normally once the it is destroyed.
enum class Owned : bool { True, False };

/// An implementation of `std::error_category` for SSL errors.
struct ErrorCategory : std::error_category
{
  const char* name() const noexcept;
  std::string message(int value) const;
};

QI_API const ErrorCategory& errorCategory();

/// Error type representing an error returned by the OpenSSL library.
/// Usually thrown from functions when an SSL error occurs.
///
/// This is based on `std::system_error`. SSL error codes (which are `unsigned long` values)
/// are translated into `std::error_code` objects associated to the error category returned by
/// `errorCategory()`.
struct QI_API Error final : std::system_error
{
  explicit Error(unsigned long err, const std::string& what = {});

  /// Returns an error containing the last SSL error code in thread's SSL error queue, and clears
  /// the queue.
  ///
  /// @see `ERR_get_error` and `ERR_clear_error`.
  static Error fromCurrentError(const std::string& what);
};

struct QI_API PemPasswordCallback
{
  using CallbackPointer = ::pem_password_cb*;

  CallbackPointer callback = nullptr;
  void* userData = nullptr;

// Regular:
  PemPasswordCallback() noexcept = default;

  friend bool operator==(const PemPasswordCallback& a,
                         const PemPasswordCallback& b)
  {
    return std::tie(a.callback, a.userData)
        == std::tie(b.callback, b.userData);
  }

  friend KA_GENERATE_REGULAR_OP_DIFFERENT(PemPasswordCallback)

  inline explicit PemPasswordCallback(CallbackPointer cb,
                                      void* userData = nullptr) noexcept
    : callback(cb)
    , userData(userData)
  {
  }

  /// Constructs a password callback from a null-terminated string.
  ///
  /// This sets the callback pointer to null and the user data to the
  /// null-terminated string.
  ///
  /// This is according to the documentation of this callback:
  ///   If the cb parameters is set to NULL and the u parameter is not NULL then
  ///   the u parameter is interpreted as a null terminated string to use as the
  ///   passphrase.
  ///
  /// @see `PEM_read_bio_X509`
  QI_API
  static PemPasswordCallback fromPassphrase(const char* passphrase) noexcept;
};

namespace detail
{

/// A type defining traits for a OpenSSL object type.
///
/// Specializations of this type must define the following members:
///   with ObjectTraits<O> T:
///     - `static void T::free(O*) noexcept`
///       Releases an SSL object, freeing it if it was the last reference to the object.
///       It must do nothing if `p` is null.
///     - `static bool T::upRef(O* p) noexcept`
///       Increments the reference count of an SSL object. Returns true if it
///       succeeded or false otherwise. It must do nothing and return true
///       if `p` is null.
///     - `static const char* T::name() noexcept`
///       Returns null terminated string of the name of SSL object type.
///       It must not return an null pointer.
template<typename O>
struct ObjectTraits;

/// Frees an OpenSSL object, according to its traits object.
template<typename O>
struct ObjectFree
{
// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_0(ObjectFree)

  using Traits = ObjectTraits<O>;

// Procedure:
  void operator()(O* p) const noexcept
  {
    Traits::free(p);
  }
};

/// A pointer to an OpenSSL object.
///
/// Since OpenSSL objects are implemented as shared reference with reference counting, objects
/// of this type are copyable. When copied, the reference count of the OpenSSL object is
/// incremented, using the `upRef` function of the objects traits (see `ObjectTraits`).
/// The `free` function of the objects traits is called when the pointer is destroyed to release the
/// reference it holds to the OpenSSL object.
///
/// Objects of this type always own the OpenSSL object it points to. When constructed from a borrowed
/// pointer of an existing OpenSSL object, the reference of the object is incremented before being
/// stored in this object, effectively transforming the borrowing pointer into an owning pointer.
template<typename O>
struct Pointer
{
protected:
  using Traits = ObjectTraits<O>;
  using UniquePtr = std::unique_ptr<O, ObjectFree<O>>;
  UniquePtr p;

public:
  using element_type = typename UniquePtr::element_type;
  using pointer      = typename UniquePtr::pointer;
  using deleter_type = typename UniquePtr::deleter_type;

  constexpr Pointer() noexcept = default;
  constexpr Pointer(std::nullptr_t) noexcept : p(nullptr) {}

  /// Constructs this object from a pointer to the OpenSSL object.
  /// If the ownership is `Ownership::Borrowed`, the reference count of the OpenSSL object is
  /// incremented before it is stored in this object.
  explicit Pointer(pointer p, Owned owned = Owned::True);

  Pointer(const Pointer& o);

  Pointer& operator=(const Pointer& o);

  /// @post `o == nullptr`
  Pointer(Pointer&& o) noexcept = default;

  /// @post `o == nullptr`
  Pointer& operator=(Pointer&& o) noexcept = default;

protected: // This type is only destructible from subtypes.
  ~Pointer() = default;

public:
  /// Increments the reference count of the OpenSSL object.
  /// Does nothing if the pointer is null.
  /// @throws `Error` if the reference count could not be incremented.
  void upRef() const;

// Interface of `std::unique_ptr`:
  explicit operator bool() const noexcept { return bool(p); }
  pointer get() const noexcept { return p.get(); }
  void reset(pointer op = pointer()) noexcept { return p.reset(op); }
  pointer release() noexcept { return p.release(); }
  void swap(Pointer& o) noexcept { return p.swap(o.p); }

  KA_GENERATE_FRIEND_REGULAR_OPS_1(Pointer, p)

  friend bool operator==(const Pointer& x, std::nullptr_t) noexcept { return x.p     == nullptr; }
  friend bool operator==(std::nullptr_t, const Pointer& x) noexcept { return nullptr == x.p;     }
  friend bool operator!=(const Pointer& x, std::nullptr_t) noexcept { return x.p     != nullptr; }
  friend bool operator!=(std::nullptr_t, const Pointer& x) noexcept { return nullptr != x.p;     }
  friend bool operator< (const Pointer& x, std::nullptr_t) noexcept { return x.p     <  nullptr; }
  friend bool operator< (std::nullptr_t, const Pointer& x) noexcept { return nullptr <  x.p;     }
  friend bool operator<=(const Pointer& x, std::nullptr_t) noexcept { return x.p     <= nullptr; }
  friend bool operator<=(std::nullptr_t, const Pointer& x) noexcept { return nullptr <= x.p;     }
  friend bool operator> (const Pointer& x, std::nullptr_t) noexcept { return x.p     >  nullptr; }
  friend bool operator> (std::nullptr_t, const Pointer& x) noexcept { return nullptr >  x.p;     }
  friend bool operator>=(const Pointer& x, std::nullptr_t) noexcept { return x.p     >= nullptr; }
  friend bool operator>=(std::nullptr_t, const Pointer& x) noexcept { return nullptr >= x.p;     }
};

template<>
struct ObjectTraits<::BIO>
{
  KA_GENERATE_FRIEND_REGULAR_OPS_0(ObjectTraits)
  using Type = ::BIO;
  // `BIO_vfree` does nothing if the pointer is null.
  static inline void free(Type* bio) noexcept  { ::BIO_vfree(bio); }
  // `BIO_up_ref` returns 1 for success and 0 for failure.
  // It is unspecified whether or not `BIO_up_ref` does nothing if the pointer is null,
  // therefore we have to handle the case manually.
  static inline bool upRef(Type* bio) noexcept { return bio == nullptr || ::BIO_up_ref(bio) == 1; }
  static inline const char* name() noexcept    { return "BIO"; }
};

template<>
struct ObjectTraits<::EVP_PKEY>
{
  KA_GENERATE_FRIEND_REGULAR_OPS_0(ObjectTraits)
  using Type = ::EVP_PKEY;
  // `EVP_PKEY_free` does nothing if the pointer is null.
  static inline void free(Type* pkey) noexcept  { ::EVP_PKEY_free(pkey); }
  // `EVP_PKEY_up_ref` returns 1 for success and 0 for failure.
  // It is unspecified whether or not `EVP_PKEY_up_ref` does nothing if the pointer is null,
  // therefore we have to handle the case manually.
  static inline bool upRef(Type* pkey) noexcept { return pkey == nullptr || ::EVP_PKEY_up_ref(pkey) == 1; }
  static inline const char* name() noexcept     { return "EVP_PKEY"; }
};

template<>
struct ObjectTraits<::X509>
{
  KA_GENERATE_FRIEND_REGULAR_OPS_0(ObjectTraits)
  using Type = ::X509;
  // `X509_free` does nothing if the pointer is null.
  static inline void free(Type* cert) noexcept  { ::X509_free(cert); }
  // `X509_up_ref` returns 1 for success and 0 for failure.
  // It is unspecified whether or not `X509_up_ref` does nothing if the pointer is null,
  // therefore we have to handle the case manually.
  static inline bool upRef(Type* cert) noexcept { return cert == nullptr || ::X509_up_ref(cert) == 1; }
  static inline const char* name() noexcept     { return "X509"; }
};

template<>
struct ObjectTraits<::X509_STORE>
{
  KA_GENERATE_FRIEND_REGULAR_OPS_0(ObjectTraits)
  using Type = ::X509_STORE;
  // It is unspecified whether or not `X509_STORE_free` does nothing if the pointer is null,
  // therefore we have to handle the case manually.
  static inline void free(Type* store) noexcept  { if (store) ::X509_STORE_free(store); }
  // `X509_STORE_up_ref` returns 1 for success and 0 for failure.
  // It is unspecified whether or not `X509_STORE_up_ref` does nothing if the pointer is null,
  // therefore we have to handle the case manually.
  static inline bool upRef(Type* store) noexcept { return store == nullptr || ::X509_STORE_up_ref(store) == 1; }
  static inline const char* name() noexcept      { return "X509_STORE"; }
};

} // namespace detail

/// An OpenSSL basic input/output pointer.
struct QI_API BIO : detail::Pointer<::BIO>
{
private:
  using Base = detail::Pointer<::BIO>;

public:
  using Pointer::Pointer;

  BIO(const BIO& o) = default;
  BIO(BIO&& o) noexcept = default;

  BIO& operator=(const BIO& o) = default;
  BIO& operator=(BIO&& o) noexcept = default;

  /// Constructs a BIO from a range of two contiguous iterators.
  ///
  /// The behavior is undefined if iterators are random access but not contiguous.
  /// The data is NOT copied, so it must be unchanged and kept valid until the
  /// BIO object is destroyed.
  ///
  /// @see `BIO_new_mem_buf`
  /// @throws `Error` if the BIO could not be created.
  ///
  /// ContiguousIterator ContiIt
  template<typename ContiIt,
           // The contiguous iterator category does not exist in C++11 (it was added in C++20).
           ka::EnableIf<ka::IsRandomAccessIterator<ContiIt>::value, bool> = true>
  static BIO fromRange(ContiIt begin, ContiIt end)
  {
    return fromRangeImpl(begin, end);
  }

  /// Trap overload for iterators that are not contiguous.
  ///
  /// Does nothing and returns nothing.
  ///
  /// Iterator It
  template<typename It,
           // The contiguous iterator category does not exist in C++11 (it was added in C++20).
           ka::EnableIf<!ka::IsRandomAccessIterator<It>::value, bool> = true>
  static void fromRange(It, It) noexcept {}

  /// Equivalent to `fromRange(begin(range), end(range))`.
  ///
  /// Linearizable (of underlying contiguous iterators) Range
  template<typename Range>
  static BIO fromRange(Range&& range)
  {
    using std::begin; using std::end;
    // Don't call `fromRangeImpl` directly, as only `fromRange(It, It)` checks
    // for the correct kind of iterators.
    return fromRange(begin(range), end(range));
  }

  /// Constructs a BIO from a file, opening it according to the given mode.
  ///
  /// @see `BIO_new_file`
  /// @throws `Error` if the BIO could not be created.
  QI_API
  static BIO fromFile(boost::filesystem::path path, const char* mode = "r");

  /// Returns the memory data for this BIO.
  ///
  /// Only effective on BIO created from memory (such as `BIO::fromRange`).
  ///
  /// @see `BIO_get_mem_data`
  std::vector<char> memoryData() const;

  /// Reads at most `size` bytes of data from this BIO.
  ///
  /// @see `BIO_read`
  std::vector<char> read(int size);

  /// Reads the first X509 certificate in PEM format found in this BIO.
  /// If `trusted` is set, reads a trusted X509 certificate instead.
  ///
  /// The function silently ignores any other element in the PEM content.
  ///
  /// An optional PEM password callback may be passed to decrypt the content of the PEM.
  ///
  /// @see `PEM_read_bio_X509` and `PEM_read_bio_X509_AUX`
  /// @throws `Error` if a X509 certificate header was found but the content could not be read.
  boost::optional<Certificate> readPemX509(bool trusted = false, PemPasswordCallback passwordCb = {});

  /// Reads the first private key in PEM format found in this BIO.
  /// The type of the key is automatically deduced.
  ///
  /// The function silently ignores any other element in the PEM content.
  ///
  /// An optional PEM password callback may be passed to decrypt the content of the PEM.
  ///
  /// @see `PEM_read_bio_PrivateKey`.
  /// @throws `Error` if a private key header was found but the content could not be read.
  boost::optional<PKey> readPemPrivateKey(PemPasswordCallback passwordCb = {});

  /// Reads a whole X509 certificate chain in PEM format from this BIO.
  ///
  /// The function silently ignores any other element in the PEM content.
  ///
  /// The first certificate found will be read as a trusted certificate (see
  /// `PEM_read_bio_X509_AUX`) but the ones found after that will be read normally
  /// (see `PEM_read_bio_X509`). This mimics the behavior of `SSL_use_certificate_chain_file`.
  ///
  /// An optional PEM password callback may be passed to decrypt the content of the PEM.
  /// @throws `Error` if a X509 certificate header was found but the content could not be read.
  std::vector<Certificate> readPemX509Chain(PemPasswordCallback passwordCb = {});

private:
  /// ContiguousIterator ContiIt
  template<typename ContiIt>
  static BIO fromRangeImpl(ContiIt begin, ContiIt end);
};

/// An OpenSSL key pointer in the form of a `EVP_PKEY` handle.
struct QI_API PKey : detail::Pointer<::EVP_PKEY>
{
private:
  using Base = detail::Pointer<::EVP_PKEY>;

public:
  using Pointer::Pointer;

  PKey(const PKey& o) = default;
  PKey(PKey&& o) noexcept = default;

  PKey& operator=(const PKey& o) = default;
  PKey& operator=(PKey&& o) noexcept = default;

  /// Reads the first private key in PEM PKCS#8 format found in the file.
  /// The type of the key is automatically deduced.
  ///
  /// An optional PEM password callback may be passed to decrypt the content of the PEM.
  ///
  /// @throws `Error` if the file could not be read or if a private key header was found but the
  /// content could not be read.
  QI_API
  static inline boost::optional<PKey> privateFromPemFile(boost::filesystem::path path,
                                                         const char* mode = "r",
                                                         PemPasswordCallback cb = {})
  {
    return BIO::fromFile(std::move(path), mode).readPemPrivateKey(std::move(cb));
  }

  /// Iterator range version of `privateFromPemFile`.
  ///
  /// ContiguousIterator ContiIt
  template <typename ContIt>
  static boost::optional<PKey> privateFromPemRange(ContIt begin,
                                                   ContIt end,
                                                   PemPasswordCallback cb = {})
  {
    return BIO::fromRange(std::move(begin), std::move(end)).readPemPrivateKey(std::move(cb));
  }

  /// Equivalent to `privateFromPemRange(begin(range), end(range), cb)`.
  ///
  /// Linearizable (of underlying contiguous iterators) Range
  template <typename Range>
  static boost::optional<PKey> privateFromPemRange(Range&& range,
                                                   PemPasswordCallback cb = {})
  {
    using std::begin; using std::end;
    return privateFromPemRange(begin(range), end(range), std::move(cb));
  }

  /// Compares the public and private key components and parameters (if
  /// present) of two keys.
  ///
  /// Returns 1 if the keys match, 0 if they don't match, -1 if the key types
  /// are different and -2 if the operation is not supported.
  ///
  /// This behavior differs from the comparison of two keys using `operator==`,
  /// which compares the value of pointers and not the content of the pointees.
  ///
  /// @see `EVP_PKEY_eq`
  int eq(const PKey& o) const;

  /// Loads this key into the SSL context.
  /// It must refer to a private key.
  ///
  /// @see `SSL_CTX_use_PrivateKey`
  /// @throws `Error` if the operation fails.
  void usePrivateIn(::SSL_CTX& ctx) const;
};

/// A X509 certificate pointer.
struct QI_API Certificate : detail::Pointer<::X509>
{
private:
  using Base = detail::Pointer<::X509>;

public:
  using Pointer::Pointer;

  Certificate(const Certificate& o) = default;
  Certificate(Certificate&& o) noexcept = default;

  Certificate& operator=(const Certificate& o) = default;
  Certificate& operator=(Certificate&& o) noexcept = default;

  /// Reads the first X509 certificate in PEM format found in the file.
  /// If `trusted` is set, reads a trusted X509 certificate instead.
  ///
  /// An optional PEM password callback may be passed to decrypt the content of the PEM.
  ///
  /// @throws `Error` if the file could not be read or if a X509 certificate header was found but
  /// the content could not be read.
  QI_API
  static inline boost::optional<Certificate> fromPemFile(boost::filesystem::path path,
                                                         const char* mode = "r",
                                                         bool trusted = false,
                                                         PemPasswordCallback cb = {})
  {
    return BIO::fromFile(std::move(path), mode).readPemX509(trusted, cb);
  }

  /// Reads a whole X509 certificate chain in PEM format from a file.
  ///
  /// The first certificate found will be read as a trusted certificate (see
  /// `PEM_read_bio_X509_AUX`) but the ones found after that will be read normally
  /// (see `PEM_read_bio_X509`). This mimics the behavior of `SSL_use_certificate_chain_file`.
  ///
  /// An optional PEM password callback may be passed to decrypt the content of the PEM.
  /// @throws `Error` if a X509 certificate header was found but the content could not be read.
  QI_API
  static inline std::vector<Certificate> chainFromPemFile(boost::filesystem::path path,
                                                          PemPasswordCallback cb = {})
  {
    return BIO::fromFile(std::move(path)).readPemX509Chain(std::move(cb));
  }

  /// Iterator range version of `fromPemFile`.
  ///
  /// ContiguousIterator ContiIt
  template<typename ContIt>
  static boost::optional<Certificate> fromPemRange(ContIt begin,
                                                   ContIt end,
                                                   bool trusted = false,
                                                   PemPasswordCallback cb = {})
  {
    return BIO::fromRange(std::move(begin), std::move(end)).readPemX509(trusted, std::move(cb));
  }

  /// Equivalent to `fromPemRange(begin(range), end(range), trusted, cb)`.
  ///
  /// Linearizable (of underlying contiguous iterators) Range
  template<typename Range>
  static boost::optional<Certificate> fromPemRange(Range&& range,
                                                   bool trusted = false,
                                                   PemPasswordCallback cb = {})
  {
    using std::begin; using std::end;
    return fromPemRange(begin(range), end(range), trusted, std::move(cb));
  }

  /// Iterator range version of `chainFromPemFile`.
  ///
  /// ContiguousIterator ContiIt
  template<typename ContIt>
  static std::vector<Certificate> chainFromPemRange(ContIt begin,
                                                    ContIt end,
                                                    PemPasswordCallback cb = {})
  {
    return BIO::fromRange(std::move(begin), std::move(end)).readPemX509Chain(std::move(cb));
  }

  /// Equivalent to `chainFromPemRange(begin(range), end(range), cb)`.
  ///
  /// Linearizable (of underlying contiguous iterators) Range
  template<typename Range>
  static std::vector<Certificate> chainFromPemRange(Range&& range,
                                                    PemPasswordCallback cb = {})
  {
    using std::begin; using std::end;
    return chainFromPemRange(begin(range), end(range), std::move(cb));
  }

  /// Returns the subject name of this certificate.
  ::X509_NAME* subjectName() const noexcept;

  /// Compares the content of two certificates.
  ///
  /// Returns an integer less, equal or greater than 0 if this object is found to be less than,
  /// to match or be greater than the other certificate, respectively.
  ///
  /// This behavior differs from the comparison of two certificates using `operator==`, which
  /// compares the value of pointers and not the content of the pointees.
  ///
  /// @see `X509_cmp`
  int cmp(const Certificate& other) const;

  /// Loads this certificate into the SSL context.
  ///
  /// @see `SSL_CTX_use_certificate`
  /// @throws `Error` if the operation fails.
  void useIn(::SSL_CTX& ctx) const;

  /// Adds this certificate to the chain of the current certificate of the SSL context.
  ///
  /// @see `SSL_CTX_add1_chain_cert`
  /// @throws `Error` if the operation fails.
  void addToCurrentChain(::SSL_CTX& ctx) const;
};

/// Returns a textual representation of the certificate.
///
/// @see `X509_print`
QI_API std::string to_string(const Certificate& cert);

/// Returns a textual representation of a certificate name (subject/issuer).
///
/// @see `X509_NAME_print`
QI_API std::string to_string(const ::X509_NAME& name);

/// Calls `to_string(cert)` and outputs the result into the stream.
QI_API std::ostream& operator<<(std::ostream& os, const Certificate& cert);

/// Calls `to_string(name)` and outputs the result into the stream.
QI_API std::ostream& operator<<(std::ostream& os, const ::X509_NAME& name);

/// A X509 certificate store pointer.
struct QI_API CertificateStore : detail::Pointer<::X509_STORE>
{
private:
  using Base = detail::Pointer<::X509_STORE>;

public:
  using Pointer::Pointer;

  CertificateStore(const CertificateStore& o) = default;
  CertificateStore(CertificateStore&& o) noexcept = default;

  CertificateStore& operator=(const CertificateStore& o) = default;
  CertificateStore& operator=(CertificateStore&& o) noexcept = default;

  /// Adds the certificate into this store.
  ///
  /// @see `X509_STORE_add_cert`
  /// @throws `Error` if the operation fails.
  void add(const Certificate& cert);

  /// Returns the VERIFY_PARAM structure of this store.
  ///
  /// @see `X509_store_get0_param`
  /// @throws `Error` if the operation fails.
  ::X509_VERIFY_PARAM& verifyParameters();
};

struct QI_API CertChainWithPrivateKey
{
// Regular:
  KA_GENERATE_FRIEND_REGULAR_OPS_2(
    CertChainWithPrivateKey,
      certificateChain,
      privateKey
  )

  /// The certificate chain, which includes the certificate of the end entity
  /// at the first position.
  std::vector<Certificate> certificateChain;

  /// The private key associated with the certificate of the end entity.
  PKey privateKey;

  /// Use this certificate chain and private key in the given SSL context.
  ///
  /// The first certificate of the chain (which should be the end entity) is set
  /// through `Certificate::useIn`, and the ones after that are added through
  /// `Certificate::addToCurrentChain`. This algorithm is based on the implementation
  /// of the `SSL_CTX_use_certificate_chain_file` function.
  ///
  /// @throws `std::runtime_error` if either the chain is empty or the private key is null.
  /// @throws `Error` if any SSL operation fails.
  void useIn(::SSL_CTX& ctx) const;
};

struct QI_API ConfigBase
{
  /// The private key and associated certificate files used to identify the local peer.
  boost::optional<CertChainWithPrivateKey> certWithPrivKey;

  /// The trusted certification authorities certificate files used to verify the
  /// remote peer certificate during TLS handshake.
  std::vector<Certificate> trustStore;

  /// If set, activates the partial chain flag in the verification. It enables
  /// intermediate CA certificates to be used as trusted root certificates, even
  /// though they are not self-signed.
  /// @see `X509_VERIFY_PARAM_set_flags` and `X509_V_FLAG_PARTIAL_CHAIN`
  bool verifyPartialChain = false;
};

struct QI_API ClientConfig : ConfigBase
{
  /// A callback to verify the certificate of the server (the peer of the client).
  ///
  /// The signature of the callback must be:
  ///   `bool callback(bool preverif, X509_STORE_CTX& ctx, const std::string& hostname)`
  /// The `preverif` boolean is set to the result of all the pre-verifications. If true,
  /// then the verifications succeeded. If false, they failed.
  /// The SSL verification context is passed as the `ctx` parameter.
  /// The `hostname` parameter is the expected host name of the server, which was given
  /// as the host in the connection URL. It is pre-checked against the name specified in
  /// the server certificate (according to RFC 6125), and the result of this check is
  /// reflected in the verifications result boolean parameter, `preverif`. The callback
  /// is free to perform additional checks against this host name and the certificate.
  ///
  /// The callback is called at least once for each certificate in the chain, starting
  /// with the trusted root and going upward to the end entity certificate. For more
  /// details, on the callback invocation, see the manual page for `SSL_set_verify`,
  /// section `verify_callback`.
  ///
  /// The callback must return a boolean, which reflects the success or failure of the
  /// verification it performs. If true, the verification is successful and the server
  /// certificate is accepted. If false, the verification is a failure and the certificate
  /// is rejected and the verification process is immediately stopped (the callback is
  /// not called for subsequent certificates of the chain).
  /// The result of the callback overrides any result of pre-verifications. Therefore it
  /// is possible to accept a certificate for which pre-verifications failed, and on the
  /// contrary reject one for which pre-verifications succeeded.
  ///
  /// @see `SSL_set_verify`
  using VerifyCallback = std::function<bool(bool, ::X509_STORE_CTX&, const std::string&)>;
  VerifyCallback verifyCallback;
};

struct QI_API ServerConfig : ConfigBase
{
  /// A callback to verify the certificate of a client (the peer of the server).
  ///
  /// The signature of the callback must be:
  ///   `bool callback(bool preverif, X509_STORE_CTX& ctx, boost::asio::ip::tcp::endpoint endpoint)`
  /// The `preverif` boolean is set to the result of all the pre-verifications. If true,
  /// then the verifications succeeded. If false, they failed.
  /// The SSL verification context is passed as the `ctx` parameter.
  /// The `endpoint` parameter is the TCP endpoint of the client.
  ///
  /// The callback is called at least once for each certificate in the chain, starting
  /// with the trusted root and going upward to the end entity certificate. For more
  /// details on the callback invocation, see the manual page for `SSL_set_verify`
  /// section `verify_callback`.
  ///
  /// The callback must return a boolean, which reflects the success or failure of the
  /// verification it performs. If true, the verification is successful and the client
  /// certificate is accepted. If false, the verification is a failure and the certificate
  /// is rejected and the verification process is immediately stopped (the callback is
  /// not called for subsequent certificates of the chain).
  /// The result of the callback overrides any result of pre-verifications. Therefore it
  /// is possible to accept a certificate for which pre-verifications failed, and on the
  /// contrary reject one for which pre-verifications succeeded.
  ///
  /// @see `SSL_set_verify`
  using VerifyCallback = std::function<bool(bool, ::X509_STORE_CTX&, boost::asio::ip::tcp::endpoint)>;
  VerifyCallback verifyCallback;
};

} // namespace ssl
} // namespace qi

#include "detail/ssl.hxx"

#endif // QI_MESSAGING_SSL_SSL_HPP

