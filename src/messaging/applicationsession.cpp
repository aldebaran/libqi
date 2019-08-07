/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <utility>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <qi/trackable.hpp>
#include <qi/applicationsession.hpp>
#include <qi/anyvalue.hpp>
#include <qi/log.hpp>
#include "applicationsession_internal.hpp"

qiLogCategory("qi.applicationsession");

namespace qi
{

namespace
{

using appsession_internal::ProgramOptions;
boost::synchronized_value<boost::optional<ProgramOptions>> g_defaultProgramOptions;

template<typename... Args>
ProgramOptions emplaceDefaultProgramOptions(Args&&... args)
{
  auto syncProgOpts = g_defaultProgramOptions.synchronize();
  syncProgOpts->emplace(std::forward<Args>(args)...);
  return syncProgOpts->get();
}

/// @throws `boost::bad_optional_access` if `setDefaultProgramOptions` was not called before.
ProgramOptions defaultProgramOptions()
{
  return g_defaultProgramOptions->value();
}

void onDisconnected(const std::string& /*errorMessage*/)
{
  Application::stop();
}

// This function is used to add the callback before the call of Application's constructor
int& addParseOptions(int& argc)
{
  Application::atEnter([]{
    const auto programOptions = emplaceDefaultProgramOptions(Application::arguments());
    Application::setArguments(programOptions.unrecognizedArgs);
    Application::options().add(ProgramOptions::description());
  });
  return argc;
}

}

class ApplicationSessionPrivate : public Trackable<ApplicationSessionPrivate>
{
public:
  ApplicationSessionPrivate(const ApplicationSession::Config& config)
    : _config(reconfigureWithProgramOptions(config, defaultProgramOptions()))
    , _session(makeSession(_config.sessionConfig()))
    , _init(false)
  {
    if (!(_config.option() & ApplicationSession::Option_NoAutoExit))
    {
      _session->disconnected.connect(&onDisconnected);
    }
  }

  virtual ~ApplicationSessionPrivate()
  {
    destroy();
    Application::stop();
    _session->close();
  }

  void start()
  {
    // Rely on the configuration we passed to the session for the URLs.

    if (_config.standalone())
    {
      _session->listenStandalone();
      return;
    }

    _session->connect();

    // Only listen if there were listen URLs specified on the command line.
    if (defaultProgramOptions().hasCliListenUrl)
      _session->listen();
  }

public:
  const ApplicationSession::Config _config;
  SessionPtr _session;
  bool _init;
  boost::mutex _mutex;
};

enum StateMachineConfig
{
  StateMachineConfig_unset = 0,
  StateMachineConfig_standAlone = 1,
  StateMachineConfig_connect = 2,
};

ApplicationSession::Config::Config()
  : _standalone(false)
  , _opt(Option_None)
{
}

ApplicationSession::Config::~Config()
{
}

ApplicationSession::Config& ApplicationSession::Config::setDefaultStandAlone(bool standalone)
{
  return setStandalone(standalone);
}

bool ApplicationSession::Config::defaultStandAlone() const
{
  return standalone();
}

ApplicationSession::Config& ApplicationSession::Config::setStandalone(bool standalone)
{
  _standalone = standalone;
  return *this;
}

bool ApplicationSession::Config::standalone() const
{
  return _standalone;
}

ApplicationSession::Config& ApplicationSession::Config::setOption(ApplicationSession::Option opt)
{
  _opt = opt;
  return *this;
}
ApplicationSession::Option ApplicationSession::Config::option() const
{
  return _opt;
}

ApplicationSession::Config& ApplicationSession::Config::setDefaultUrl(const Url& url)
{
  return setConnectUrl(url);
}

const Url& ApplicationSession::Config::defaultUrl() const
{
  return connectUrl().value();
}

ApplicationSession::Config& ApplicationSession::Config::setConnectUrl(Url url)
{
  _sessionConfig.connectUrl = std::move(url);
  return *this;
}

const boost::optional<Url>& ApplicationSession::Config::connectUrl() const
{
  return _sessionConfig.connectUrl;
}

ApplicationSession::Config& ApplicationSession::Config::setDefaultListenUrl(const Url& listenUrl)
{
  setListenUrls({ listenUrl });
  return *this;
}

const Url& ApplicationSession::Config::defaultListenUrl() const
{
  return listenUrls().front();
}

ApplicationSession::Config& ApplicationSession::Config::addListenUrl(Url listenUrl)
{
  _sessionConfig.listenUrls.push_back(std::move(listenUrl));
  return *this;
}

ApplicationSession::Config& ApplicationSession::Config::setListenUrls(std::vector<Url> listenUrls)
{
  _sessionConfig.listenUrls = std::move(listenUrls);
  return *this;
}

const std::vector<Url>&ApplicationSession::Config::listenUrls() const
{
  return _sessionConfig.listenUrls;
}

ApplicationSession::Config& ApplicationSession::Config::setSessionConfig(
  SessionConfig sessConfig)
{
  _sessionConfig = std::move(sessConfig);
  return *this;
}

const SessionConfig& ApplicationSession::Config::sessionConfig() const
{
  return _sessionConfig;
}

ApplicationSession::Config& ApplicationSession::Config::setName(const std::string& name)
{
  _name = name;
  return *this;
}
const std::string& ApplicationSession::Config::name() const
{
  return _name;
}

ApplicationSession::ApplicationSession(int& argc, char**& argv, int opt, const Url& defaultUrl)
  : Application(addParseOptions(argc), argv)
  , _p(new ApplicationSessionPrivate(
      Config{}.setConnectUrl(defaultUrl).setOption(static_cast<Option>(opt))))
{
}

ApplicationSession::ApplicationSession(const std::string& name,
                                       int& argc,
                                       char**& argv,
                                       int opt,
                                       const Url& defaultUrl)
  : Application(addParseOptions(argc), argv, name)
  , _p(new ApplicationSessionPrivate(
      Config{}.setName(name).setConnectUrl(defaultUrl).setOption(static_cast<Option>(opt))))
{
}

ApplicationSession::ApplicationSession(int& argc, char**& argv, const Config& defaultConfig)
  : Application(addParseOptions(argc), argv, defaultConfig.name())
  , _p(new ApplicationSessionPrivate(defaultConfig))
{
}

ApplicationSession::~ApplicationSession() = default;

SessionPtr ApplicationSession::session() const
{
  return _p->_session;
}

const ApplicationSession::Config& ApplicationSession::config() const
{
  return _p->_config;
}

Url ApplicationSession::url() const
{
  const auto connectUrl = _p->_config.connectUrl();
  // The configuration must hold a connect URL as it is guaranteed by `finalizeConfig`.
  QI_ASSERT_TRUE(connectUrl.is_initialized());
  return *_p->_config.connectUrl();
}

Url ApplicationSession::listenUrl() const
{
  const auto& confListenUrls = _p->_config.sessionConfig().listenUrls;
  QI_ASSERT_FALSE(confListenUrls.empty());
  return confListenUrls.at(0);
}

std::vector<Url> ApplicationSession::allListenUrl() const
{
  return _p->_config.sessionConfig().listenUrls;
}

void ApplicationSession::start()
{
  startSession();
}

void ApplicationSession::startSession()
{
  {
    boost::mutex::scoped_lock lock(_p->_mutex);

    if (_p->_init)
    {
      return;
    }
    _p->_init = true;
  }

  // The connection is asynchronous, therefore a wait is expected here
  _p->start();
}

void ApplicationSession::run()
{
  if(!_p->_session->isConnected())
    startSession();
  Application::run();
}

bool ApplicationSession::standAlone()
{
  return _p->_config.standalone();
}

std::string ApplicationSession::helpText() const
{
  return Application::helpText();
}

}
