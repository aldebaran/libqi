/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/program_options.hpp>
#include <qi/trackable.hpp>
#include <qi/applicationsession.hpp>
#include <qi/anyvalue.hpp>

static void onDisconnected(const std::string& /*errorMessage*/)
{
  ::qi::Application::stop();
}

static std::string _address;
static std::string _listenAddress;
static bool _standAlone = false;

static void parseAddress()
{
  namespace po = boost::program_options;
  po::options_description desc("ApplicationSession options");

  desc.add_options()
      ("qi-url", po::value<std::string>(&_address), "The address of the service directory")
      ("qi-listen-url", po::value<std::string>(&_listenAddress), "The url to listen to")
      ("qi-standalone", "create a standalone session (this will use qi-listen-url if provided");

  po::variables_map vm;
  po::parsed_options parsed =
      po::command_line_parser(qi::Application::arguments()).options(desc).allow_unregistered().run();
  po::store(parsed, vm);
  po::notify(vm);

  qi::Application::setArguments(po::collect_unrecognized(parsed.options, po::include_positional));
  _standAlone = vm.count("qi-standalone");

  {
    po::options_description descTmp;
    descTmp.add_options()
        ("help,h", "");

    po::variables_map vmTmp;
    po::store(po::command_line_parser(qi::Application::arguments()).options(descTmp).allow_unregistered().run(),
              vmTmp);

    if (vmTmp.count("help"))
      std::cout << desc << std::endl;
  }
}

// This function is used to add the callback before the call of Application's constructor
static int& addParseOptions(int& argc)
{
  qi::Application::atEnter(parseAddress);
  return argc;
}

namespace qi
{
class ApplicationSessionPrivate : public Trackable<ApplicationSessionPrivate>
{
public:
  ApplicationSessionPrivate(const ApplicationSession::Config& config)
    : _session(new qi::Session)
    , _init(false)
  {
    if (!(config.option() & qi::ApplicationSession::Option_NoAutoExit))
    {
      _session->disconnected.connect(&::onDisconnected);
    }

    if (_standAlone && !_address.empty())
      throw std::runtime_error("You cannot be standAlone if you specified --qi-url to connect");

    _standAlone = _standAlone ? _standAlone : config.defaultStandAlone();
    if (!_address.empty())
    {
      _url = Url(_address, "tcp", 9559);
      _standAlone = false;
    }
    else
    {
      _url = config.defaultUrl();
    }
    _listenUrl = _listenAddress.empty() ? config.defaultListenUrl() : Url(_listenAddress, "tcp", 9559);
  }

  virtual ~ApplicationSessionPrivate()
  {
    destroy();
    qi::Application::stop();
    _session->close();
  }

  void connect()
  {
    if (_standAlone)
    {
      _session->listenStandalone(_listenUrl);
      return;
    }

    // listen + connect
    _session->connect(_url);
    if (!_listenAddress.empty())
      _session->listen(_listenUrl);
  }

public:
  SessionPtr _session;
  bool _init;
  Url _url;
  Url _listenUrl;
  boost::mutex _mutex;
};

enum StateMachineConfig
{
  StateMachineConfig_unset = 0,
  StateMachineConfig_standAlone = 1,
  StateMachineConfig_connect = 2,
};

ApplicationSession::Config::Config()
  : _stateMachine(StateMachineConfig_unset)
  , _opt(Option_None)
  , _url("tcp://127.0.0.1:9559")
  , _listenUrl("tcp://0.0.0.0:9559")
{
}

ApplicationSession::Config::~Config()
{
}

ApplicationSession::Config& ApplicationSession::Config::setDefaultStandAlone(bool standAlone)
{
  if (_stateMachine == StateMachineConfig_connect)
    throw std::runtime_error("You cannot be standAlone if you specified url to connect");

  if (standAlone)
    _stateMachine = StateMachineConfig_standAlone;

  return *this;
}
bool ApplicationSession::Config::defaultStandAlone() const
{
  return _stateMachine == StateMachineConfig_standAlone;
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
  if (_stateMachine == StateMachineConfig_standAlone)
    throw std::runtime_error("You cannot specify url to connect if you are standAlone");

  _url = url;
  _stateMachine = StateMachineConfig_connect;
  return *this;
}
const Url& ApplicationSession::Config::defaultUrl() const
{
  return _url;
}

ApplicationSession::Config& ApplicationSession::Config::setDefaultListenUrl(const Url& listenUrl)
{
  _listenUrl = listenUrl;
  return *this;
}
const Url& ApplicationSession::Config::defaultListenUrl() const
{
  return _listenUrl;
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

static void envConfigInit(qi::ApplicationSession::Config& conf)
{
  std::string listenUrl = qi::os::getenv("QI_LISTEN_URL");
  std::string sdUrl = qi::os::getenv("QI_URL");

  if (listenUrl.length())
    conf.setDefaultListenUrl(Url(listenUrl));
  if (sdUrl.length() && !conf.defaultStandAlone())
    conf.setDefaultUrl(sdUrl);
}

ApplicationSession::ApplicationSession(int& argc, char**& argv, int opt, const Url& url)
  : Application(::addParseOptions(argc), argv)
{
  Config config;
  envConfigInit(config);
  config.setDefaultUrl(url);
  config.setOption((Option)opt);

  _p = new ApplicationSessionPrivate(config);
}
ApplicationSession::ApplicationSession(const std::string& name,
                                       int& argc,
                                       char**& argv,
                                       int opt,
                                       const Url& url)
  : Application(::addParseOptions(argc), argv, name)
{
  Config config;
  envConfigInit(config);
  config.setName(name);
  config.setDefaultUrl(url);
  config.setOption((Option)opt);

  _p = new ApplicationSessionPrivate(config);
}

ApplicationSession::ApplicationSession(int& argc, char**& argv, const Config& defaultConfig)
  : Application(::addParseOptions(argc), argv, defaultConfig.name())
{
  Config config(defaultConfig);
  envConfigInit(config);
  _p = new ApplicationSessionPrivate(config);
}

ApplicationSession::~ApplicationSession()
{
  delete _p;
  _p = 0;
}

SessionPtr ApplicationSession::session()
{
  return _p->_session;
}

Url ApplicationSession::url()
{
  return _p->_url;
}

Url ApplicationSession::listenUrl()
{
  return _p->_listenUrl;
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
  _p->connect();
}

void ApplicationSession::run()
{
  startSession();
  Application::run();
}
}
