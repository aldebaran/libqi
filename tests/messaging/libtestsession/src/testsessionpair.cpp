/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <testsession/testsession.hpp>
#include <testsession/testsessionpair.hpp>
#include <testssl/testssl.hpp>

qiLogCategory("test.sessionpair");

const TestSessionPair::ShareServiceDirectory_tag TestSessionPair::ShareServiceDirectory{};

TestSessionPair::TestSessionPair(TestMode::Mode mode, std::string sdUrl)
  : _mode(mode)
{
  const bool gatewayMode = _mode == TestMode::Mode_Gateway;
  std::string gwUrl;
  qi::Session::Config sdConfig;
  if (gatewayMode)
  {
    gwUrl = std::move(sdUrl);
    sdUrl = "tcp://0.0.0.0:0";
  }
  else if (_mode == TestMode::Mode_SSL)
  {
    sdConfig.serverSslConfig = test::ssl::serverConfig(test::ssl::server());
  }
  gwUrl = test::adaptScheme(gwUrl, mode);
  sdUrl = test::adaptScheme(sdUrl, mode);

  _sd = qi::makeSession(sdConfig);
  _sd->listenStandalone(std::move(sdUrl));
  const auto sdEndpoint = test::url(*_sd);
  qiLogInfo() << "ServiceDirectory listening on endpoint '" << sdEndpoint << "'";
  const auto endpoint = [&]{
    if (gatewayMode)
    {
      // The gateway accepts any client signed by its root CA.
      // We could also pin the client certificate, but we let the client
      // do that. This way we cover both cases (pinning and not pinning).
      auto serverSslConfig = test::ssl::serverConfig(test::ssl::gateway::server(),
                                                     test::ssl::gateway::rootCA());
      serverSslConfig.verifyPartialChain = false;

      qi::Gateway::Config gwConfig;
      gwConfig.serviceDirectoryUrl = sdEndpoint;
      gwConfig.listenUrls = { gwUrl };
      gwConfig.serverSslConfig = std::move(serverSslConfig);
      _gw = qi::Gateway::create(std::move(gwConfig)).value();
      const auto endpoint = test::url(*_gw);
      qiLogInfo() << "Gateway listening on endpoint '" << endpoint << "'";
      return endpoint;
    }
    else
    {
      return sdEndpoint;
    }
  }();

  qiLogInfo() << "Server and client will connect to endpoint '" << endpoint << "'";
  _server.reset(new TestSession(endpoint.str(), true, _mode));
  if (_mode != TestMode::Mode_Direct) // no client in direct mode
  {
    _client.reset(new TestSession(endpoint.str(), false, _mode));
  }
}

TestSessionPair::TestSessionPair(ShareServiceDirectory_tag,
                                 const TestSessionPair& other,
                                 TestMode::Mode mode)
  : TestSessionPair(mode == TestMode::Mode_Gateway ?
                        test::url(other.gateway()) :
                        test::url(*other.sd()),
                    mode)
{
}

TestSessionPair::TestSessionPair(const qi::Url& sdEndpoint, TestMode::Mode mode)
  : _mode(mode)
  , _server(new TestSession(sdEndpoint.str(), true, _mode))
  , _client(_mode == TestMode::Mode_Direct ? nullptr : new TestSession(sdEndpoint.str(), false, _mode))
{
  qiLogInfo() << "Server and client connected to endpoint '" << sdEndpoint.str() << "'";
}

qi::SessionPtr TestSessionPair::client() const
{
  // #0 If activated test mode is 'Direct', cheat.
  if (_mode == TestMode::Mode_Direct)
    return _server->session();

  return _client->session();
}

qi::SessionPtr TestSessionPair::server() const
{
  return _server->session();
}

qi::SessionPtr TestSessionPair::sd() const
{
  return _sd;
}

const qi::Gateway& TestSessionPair::gateway() const
{
  if (!_gw)
    throw std::runtime_error("No gateway instanciated");
  return *_gw;
}

std::vector<qi::Url> TestSessionPair::serviceDirectoryEndpoints() const
{
  return _sd->endpoints();
}

std::vector<qi::Url> TestSessionPair::gatewayEndpoints() const
{
  if (_gw)
    return _gw->endpoints();
  return std::vector<qi::Url>{};
}
