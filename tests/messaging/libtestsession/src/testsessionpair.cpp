/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <testsession/testsession.hpp>
#include <testsession/testsessionpair.hpp>

qiLogCategory("test.sessionpair");

const TestSessionPair::ShareServiceDirectory_tag TestSessionPair::ShareServiceDirectory{};

TestSessionPair::TestSessionPair(TestMode::Mode mode, std::string sdUrl)
  : _mode(mode)
  , _sd(qi::makeSession())
  , _gw(_mode == TestMode::Mode_Gateway ? new qi::Gateway : nullptr)
{
  const bool gatewayMode = _mode == TestMode::Mode_Gateway;
  std::string gwUrl;
  if (gatewayMode)
  {
    gwUrl = std::move(sdUrl);
    sdUrl = "tcp://0.0.0.0:0";
  }
  else if (_mode == TestMode::Mode_SSL)
  {
    _sd->setIdentity(qi::path::findData("qi", "server.key"),
                     qi::path::findData("qi", "server.crt"));
  }
  gwUrl = test::adaptScheme(gwUrl, mode);
  sdUrl = test::adaptScheme(sdUrl, mode);

  qi::UrlVector endpoints;
  _sd->listenStandalone(std::move(sdUrl));
  qiLogInfo() << "ServiceDirectory listening on endpoint '" << _sd->endpoints()[0].str() << "'";
  if (gatewayMode)
  {
    _gw->attachToServiceDirectory(_sd->url()).wait();
    auto listenStatus = _gw->listenAsync(std::move(gwUrl)).value();
    QI_IGNORE_UNUSED(listenStatus);
    QI_ASSERT_FALSE(listenStatus == qi::Gateway::ListenStatus::NotListening);
    qiLogInfo() << "Gateway listening on endpoint '" << _gw->endpoints()[0].str() << "'";
    endpoints = _gw->endpoints();
  }
  else
  {
    endpoints = _sd->endpoints();
  }

  qiLogInfo() << "Server and client will connect to endpoint '" << endpoints[0].str() << "'";
  _server.reset(new TestSession(endpoints[0].str(), true, _mode));
  if (_mode != TestMode::Mode_Direct) // no client in direct mode
  {
    _client.reset(new TestSession(endpoints[0].str(), false, _mode));
  }
}

TestSessionPair::TestSessionPair(ShareServiceDirectory_tag t,
                                 const TestSessionPair& other,
                                 TestMode::Mode mode)
  : TestSessionPair(mode == TestMode::Mode_Gateway ?
                        other.gateway().endpoints().front() :
                        other.sd()->endpoints().front(),
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
