/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include <qi/os.hpp>

#include <testsession/testsession.hpp>
#include <testsession/testsessionpair.hpp>

const TestSessionPair::ShareServiceDirectory_tag TestSessionPair::ShareServiceDirectory{};

TestSessionPair::TestSessionPair(TestMode::Mode mode, std::string sdUrl)
  : _mode(mode == TestMode::Mode_Default ? TestMode::getTestMode() : mode)
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
  gwUrl = test::adaptScheme(gwUrl);
  sdUrl = test::adaptScheme(sdUrl);

  qi::UrlVector endpoints;
  _sd->listenStandalone(std::move(sdUrl));
  if (gatewayMode)
  {
    _gw->attachToServiceDirectory(_sd->url()).wait();
    _gw->listen(std::move(gwUrl));
    endpoints = _gw->endpoints();
  }
  else
  {
    endpoints = _sd->endpoints();
  }

  _server.reset(new TestSession(endpoints[0].str(), true, _mode));
  if (_mode != TestMode::Mode_Direct) // no client in direct mode
  {
    _client.reset(new TestSession(endpoints[0].str(), false, _mode));
  }
}

TestSessionPair::TestSessionPair(ShareServiceDirectory_tag t, const TestSessionPair& other, TestMode::Mode mode)
  : TestSessionPair(mode == TestMode::Mode_Gateway ? other.gateway().endpoints().front() :
                                                     other.sd()->endpoints().front(), mode)
{}

TestSessionPair::TestSessionPair(const qi::Url& sdEndpoint, TestMode::Mode mode)
  : _mode(mode == TestMode::Mode_Default ? TestMode::getTestMode() : mode)
  , _server(new TestSession(sdEndpoint.str(), true, _mode))
  , _client(_mode == TestMode::Mode_Direct ? nullptr : new TestSession(sdEndpoint.str(), false, _mode))
{
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
