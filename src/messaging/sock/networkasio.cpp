/*
**  Copyright (C) 2022 Aldebaran Robotics
**  See COPYING for the license
*/

#include "networkasio.hpp"
#include <ka/macro.hpp>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <boost/predef/os.h>
#include <boost/asio/ip/tcp.hpp>

namespace qi
{
namespace sock
{

namespace detail
{
namespace
{

void setVerifyParamFlags(X509_VERIFY_PARAM& param, bool set, unsigned long flags)
{
  ::ERR_clear_error();

  // X509_VERIFY_PARAM_set_flags(), X509_VERIFY_PARAM_clear_flags() [...] return 1 for success and
  // 0 for failure.
  int result = 0;
  if (set)
    result = ::X509_VERIFY_PARAM_set_flags(&param, flags);
  else
    result = ::X509_VERIFY_PARAM_clear_flags(&param, flags);
  if (result == 0)
    throw ssl::Error::fromCurrentError("could not set/reset flags on X509_VERIFY_PARAM object");
}

struct VerifyCertificateHandleException
{
  std::string logCategory;

KA_WARNING_PUSH()
KA_WARNING_DISABLE(,unused-function)
  KA_GENERATE_FRIEND_REGULAR_OPS_1(
    VerifyCertificateHandleException,
      logCategory
  );
KA_WARNING_POP()

  template<typename... E>
  bool operator()(E&&... ex) const noexcept
  {
    const auto err = ka::exception_message_t{}(std::forward<E>(ex)...);
    qiLogWarning(logCategory.c_str()) << "Exception thrown during verification: " << err;
    return false;
  }
};

/// Verification callback for client authentication of a server peer.
struct ClientVerifyCertificate
{
  ssl::ClientConfig::VerifyCallback cb;
  std::string expectedServerName;

  bool operator()(bool ok, boost::asio::ssl::verify_context& ctx)
  {
    return ka::invoke_catch(
      VerifyCertificateHandleException{ "qi.messaging.ssl.verify.client" },
      [&]{
        const auto hostnameVerif = boost::asio::ssl::host_name_verification(expectedServerName);
        ok = hostnameVerif(ok, ctx);
        if (cb)
          ok = cb(ok, *ctx.native_handle(), expectedServerName);
        return ok;
      }
    );
  }
};

/// Verification callback for server authentication of a client peer.
struct ServerVerifyCertificate
{
  ssl::ServerConfig::VerifyCallback cb;
  boost::asio::ip::tcp::endpoint clientEndpoint;

  bool operator()(bool ok, boost::asio::ssl::verify_context& ctx)
  {
    return ka::invoke_catch(
      VerifyCertificateHandleException{ "qi.messaging.ssl.verify.server" },
      [&]{
        if (cb)
          ok = cb(ok, *ctx.native_handle(), clientEndpoint);
        return ok;
      }
    );
  }
};

void apply(boost::asio::ssl::context& ctx, const ssl::ConfigBase& cfg)
{
  auto& nativeCtx = *ctx.native_handle();

  if (cfg.certWithPrivKey)
    cfg.certWithPrivKey->useIn(nativeCtx);

  auto store = ssl::CertificateStore(::SSL_CTX_get_cert_store(&nativeCtx), ssl::Owned::False);
  for (const auto& cert : cfg.trustStore)
    store.add(cert);
  setVerifyParamFlags(store.verifyParameters(), cfg.verifyPartialChain, X509_V_FLAG_PARTIAL_CHAIN);
}

} // anonymous namespace
} // namespace detail

// Note:
// -----
// In the following functions, it would be more generic to directly take a `SSL_CTX*` and configure
// it ourself, but the `boost::asio::ssl::context` may set custom data on the SSL context that it
// will try to delete in its own destructor. This means that we cannot set these custom data fields
// ourselves on the SSL context, otherwise the Boost.Asio context object might delete them, which
// would be undefined behavior. Consequently, it is unsafe to try to handle the SSL context outside
// of the Boost.Asio context object.

void NetworkAsio::applyConfig(ssl_context_type& context,
                              const ssl::ServerConfig& cfg,
                              boost::asio::ip::tcp::endpoint clientEndpoint)
{
  detail::apply(context, cfg);
  context.set_verify_callback(
    detail::ServerVerifyCertificate{ cfg.verifyCallback, std::move(clientEndpoint) });
}

void NetworkAsio::applyConfig(ssl_context_type& context,
                              const ssl::ClientConfig& cfg,
                              std::string expectedServerName)
{
  detail::apply(context, cfg);
  context.set_verify_callback(
    detail::ClientVerifyCertificate{ cfg.verifyCallback, std::move(expectedServerName) });
}

} // namespace sock
} // namespace qi
