/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <utility>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <qi/trackable.hpp>
#include <qi/applicationsession.hpp>
#include <qi/anyvalue.hpp>
#include <qi/log.hpp>

qiLogCategory("qi.applicationsession");

namespace qi
{

namespace
{

void onDisconnected(const std::string& /*errorMessage*/)
{
  Application::stop();
}

std::string& argvConnectAddress()
{
  static std::string address;
  return address;
}

std::string& argvListenAddresses()
{
  static std::string addresses;
  return addresses;
}

bool& argvStandalone()
{
  static bool standalone = false;
  return standalone;
}

void parseAddress()
{
  namespace po = boost::program_options;
  po::options_description desc("ApplicationSession options");

  static const std::string qiListenUrlsOption = ""
      "Set URL to listen to.\n"
      "Can be more than one URL to listen semicolon-separated list.\n"

      " tcp://127.0.0.1:9555;tcp://:9999;127.0.0.1\n"
      "Missing information from incomplete URLs will be defaulted by defaultListenUrl.\n"
      "If the default URL is tcps://0.0.0.0:9559 the previous list will become:\n"
      " tcp://127.0.0.1:9555;tcp://0.0.0.0:9999;tcps://127.0.0.1:9559";

  desc.add_options()
      ("qi-url", po::value<std::string>(&argvConnectAddress()), "The address of the service directory")
      ("qi-listen-url", po::value<std::string>(&argvListenAddresses()), qiListenUrlsOption.c_str())
      ("qi-standalone", "create a standalone session (this will use qi-listen-url if provided");

  po::variables_map vm;
  po::parsed_options parsed =
      po::command_line_parser(Application::arguments()).options(desc).allow_unregistered().run();
  po::store(parsed, vm);
  po::notify(vm);

  Application::setArguments(po::collect_unrecognized(parsed.options, po::include_positional));
  argvStandalone() = vm.count("qi-standalone") ? true : false;

  Application::options().add(desc);
}

// This function is used to add the callback before the call of Application's constructor
int& addParseOptions(int& argc)
{
  Application::atEnter(parseAddress);
  return argc;
}

/// @pre `addressesStr` is not empty.
/// @post The returned list is not empty.
std::vector<Url> splitListenAddresses(const std::string& addressesStr,
                                      const Url& base = SessionConfig::defaultListenUrl())
{
  QI_ASSERT_FALSE(addressesStr.empty());
  std::vector<std::string> addresses;
  boost::split(addresses, addressesStr, boost::is_any_of(";"));
  const auto urls = boost::adaptors::transform(addresses, [&](const std::string& address) {
    return specifyUrl(Url(address), base);
  });
  QI_ASSERT_FALSE(urls.empty());
  return std::vector<Url>(urls.begin(), urls.end());
}

/// @post The returned configuration holds a valid connect URL and a least one valid listen URL.
ApplicationSession::Config finalizeConfig(ApplicationSession::Config conf)
{
  // Priority for configuration values are: command line over environment over hardcoded.
  // Assuming hardcoded values are already in the config passed as the parameter, we first check
  // the environment values and then the command line arguments.
  //
  // At each step, we keep the first of the listen URLs with lower priorities as base for incomplete
  // URLs before discarding all lower priorities URLs.

  const auto updateConnectUrl = [&](const std::string& connectAddress) {
    if (connectAddress.empty())
      return;

    auto base = conf.connectUrl().value_or(Url());
    base = specifyUrl(base, SessionConfig::defaultConnectUrl());

    qiLogVerbose() << "Connect url specified: " << connectAddress
                   << ", now defaulting missing URL parts from " << base;
    conf.setConnectUrl(specifyUrl(Url(connectAddress), base));
    qiLogVerbose() << "Connect url is now: " << *conf.connectUrl();
  };

  const auto updateListenUrls = [&](const std::string& listenAddresses) {
    if (listenAddresses.empty())
      return;

    const auto& confListenUrls = conf.listenUrls();
    auto base =
      confListenUrls.empty() ? Url{} : confListenUrls.front();
    base = specifyUrl(base, SessionConfig::defaultListenUrl());

    qiLogVerbose() << "Listen URLs specified: {" << listenAddresses
                 << "}, now defaulting missing URL parts with " << base;

    auto listenUrls = splitListenAddresses(listenAddresses, base);
    conf.setListenUrls(listenUrls);

    auto listenUrlsStr = boost::join(
      boost::adaptors::transform(conf.listenUrls(), [](const Url& url) { return url.str(); }), " ");
    qiLogVerbose() << "Listen URLs are now: " << listenUrlsStr;
  };

  qiLogVerbose() << "Interpreting environment variables.";
  {
    const auto envConnectAddress = os::getenv("QI_URL");
    const auto envListenAddresses = os::getenv("QI_LISTEN_URL");
    updateConnectUrl(envConnectAddress);
    updateListenUrls(envListenAddresses);
  }

  qiLogVerbose() << "Interpreting command line arguments.";

  // Having both standalone and a connect URL as command line arguments is not acceptable.
  if (argvStandalone() && !argvConnectAddress().empty())
    throw std::runtime_error("You cannot specify both --qi-standalone and --qi-url to connect.");

  conf.setStandalone(conf.standalone() || argvStandalone());
  updateConnectUrl(argvConnectAddress());
  updateListenUrls(argvListenAddresses());

  return conf;
}

}

class ApplicationSessionPrivate : public Trackable<ApplicationSessionPrivate>
{
public:
  ApplicationSessionPrivate(const ApplicationSession::Config& config)
    : _config(finalizeConfig(config))
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
    if (!argvListenAddresses().empty())
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
  return _p->_config.sessionConfig().listenUrls.at(0);
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
  return argvStandalone();
}

std::string ApplicationSession::helpText() const
{
  return Application::helpText();
}
}
