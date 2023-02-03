/*
**  Copyright (C) 2022 Aldebaran Robotics
**  See COPYING for the license
*/

#ifndef TESTS_LIBTESTSSL_TESTSSL_HPP
#define TESTS_LIBTESTSSL_TESTSSL_HPP
#pragma once

#include <boost/optional.hpp>
#include <qi/messaging/ssl/ssl.hpp>

namespace test
{
namespace ssl
{

///                       ╔═══════════╗                             ╔═══════════╗
///                       ║ RootCA_A  ║                             ║ RootCA_B  ║
///                       ╚═════╤═════╝                             ╚═════╤═════╝
///              ┌──────────────┴─────────────┐                    ┌──────┴──────┐
///              ▼                            ▼                    ▼             ▼
///          ╔═══════╗                    ╔═══════╗          ╔══════════╗   ╔══════════╗
///          ║ CA_A1 ║                    ║ CA_A2 ║          ║ Server B ║   ║ Client B ║
///          ╚═══╤═══╝                    ╚═══╤═══╝          ╚══════════╝   ╚══════════╝
///       ┌──────┴──────┐              ┌──────┴──────┐
///       ▼             ▼              ▼             ▼
/// ╔═══════════╗ ╔═══════════╗  ╔═══════════╗ ╔═══════════╗
/// ║ Server A1 ║ ║ Client A1 ║  ║ Server A2 ║ ║ Client A2 ║
/// ╚═══════════╝ ╚═══════════╝  ╚═══════════╝ ╚═══════════╝

inline qi::ssl::Certificate endEntity(const qi::ssl::CertChainWithPrivateKey& cwpk)
{
  return cwpk.certificateChain.at(0);
}

// By default, server/client certificates are the ones from chain A1.
// For simplification purposes, they should be accessible through `test::ssl::server`,
// `test::ssl:client`, which is why the two namespaces are inline.
inline namespace a
{
  const qi::ssl::CertChainWithPrivateKey& rootCA();

  inline namespace one
  {
    const qi::ssl::CertChainWithPrivateKey& ca();
    const qi::ssl::CertChainWithPrivateKey& server();
    const qi::ssl::CertChainWithPrivateKey& client();
  }

  namespace two
  {
    const qi::ssl::CertChainWithPrivateKey& ca();
    const qi::ssl::CertChainWithPrivateKey& server();
    const qi::ssl::CertChainWithPrivateKey& client();
  }
}
namespace b
{
  const qi::ssl::CertChainWithPrivateKey& rootCA();
  const qi::ssl::CertChainWithPrivateKey& server();
  const qi::ssl::CertChainWithPrivateKey& client();
}

// Gateways in tests use the chain B certificates.
namespace gateway = b;

template<typename Cfg>
Cfg config(boost::optional<const qi::ssl::CertChainWithPrivateKey&> identity = {},
           boost::optional<const qi::ssl::CertChainWithPrivateKey&> trust = {})
{
  Cfg cfg;
  cfg.certWithPrivKey = identity;
  if (trust.has_value())
    cfg.trustStore = { endEntity(*trust) };
  return cfg;
}

template<typename... Args>
qi::ssl::ClientConfig clientConfig(Args&&... args) { return config<qi::ssl::ClientConfig>(std::forward<Args>(args)...); }
template<typename... Args>
qi::ssl::ServerConfig serverConfig(Args&&... args) { return config<qi::ssl::ServerConfig>(std::forward<Args>(args)...); }

std::string getServerA1CertsChain();
std::string getServerA1Key();
std::string getClientA1CertsChain();

} // namespace ssl
} // namespace test

#endif // TESTS_LIBTESTSSL_TESTSSL_HPP
