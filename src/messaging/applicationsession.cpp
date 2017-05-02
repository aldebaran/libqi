/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <qi/trackable.hpp>
#include <qi/applicationsession.hpp>
#include <qi/anyvalue.hpp>
#include <qi/log.hpp>

qiLogCategory("qi.applicationsession");

static void onDisconnected(const std::string& /*errorMessage*/)
{
  ::qi::Application::stop();
}


std::string& address()
{
  static std::string address;
  return address;
}

std::string& listenAddresses()
{
  static std::string addresses;
  return addresses;
}

bool& standAlone()
{
  static bool standalone = false;
  return standalone;
}

static void parseAddress()
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
      ("qi-url", po::value<std::string>(&address()), "The address of the service directory")
      ("qi-listen-url", po::value<std::string>(&listenAddresses()), qiListenUrlsOption.c_str())
      ("qi-standalone", "create a standalone session (this will use qi-listen-url if provided");

  po::variables_map vm;
  po::parsed_options parsed =
      po::command_line_parser(qi::Application::arguments()).options(desc).allow_unregistered().run();
  po::store(parsed, vm);
  po::notify(vm);

  qi::Application::setArguments(po::collect_unrecognized(parsed.options, po::include_positional));
  standAlone() = vm.count("qi-standalone") ? true : false;

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

    bool& standalone = standAlone();
    std::string& addr = address();
    if (standalone && !addr.empty())
      throw std::runtime_error("You cannot be standAlone if you specified --qi-url to connect");

    standalone = standalone ? standalone : config.defaultStandAlone();
    if(!addr.empty())
      standalone = false;

    qiLogDebug() << "Connect url specified was: " << addr << ", now defaulting missing url parts from "
                 << config.defaultUrl().str();
    _url = specifyUrl(Url(addr), config.defaultUrl());
    qiLogDebug() << "Connect url is now: " << _url.str();


    std::vector<std::string> listenUrls;
    boost::split(listenUrls, listenAddresses(), boost::is_any_of(";"));
    for (const std::string& url : listenUrls)
      _listenUrls.push_back(specifyUrl(Url(url), config.defaultListenUrl()));

    if (!_listenUrls.empty())
    {
      qiLogDebug() << "Listen url specified: "
                   << boost::algorithm::join(listenUrls, ", ")
                   << ", now defaulting missing url parts with "
                   << config.defaultListenUrl().str();


      std::ostringstream ssListenUrl;
      for (const auto& url : _listenUrls)
        ssListenUrl << " " << url.str();
      qiLogDebug() << "Listen url are now:" << ssListenUrl.str();
    }
  }

  virtual ~ApplicationSessionPrivate()
  {
    destroy();
    qi::Application::stop();
    _session->close();
  }

  void connect()
  {
    if (standAlone())
    {
      _session->listenStandalone(_listenUrls);
      return;
    }

    // listen + connect
    _session->connect(_url);
    if (!listenAddresses().empty())
    {
      for (const qi::Url& listenUrl : _listenUrls)
        _session->listen(listenUrl);
    }
  }

public:
  SessionPtr _session;
  bool _init;
  Url _url;
  std::vector<Url> _listenUrls;
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

SessionPtr ApplicationSession::session() const
{
  return _p->_session;
}

Url ApplicationSession::url() const
{
  return _p->_url;
}

Url ApplicationSession::listenUrl() const
{
  return _p->_listenUrls.at(0);
}

std::vector<Url> ApplicationSession::allListenUrl() const
{
  return _p->_listenUrls;
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
  if(!_p->_session->isConnected())
    startSession();
  Application::run();
}

bool ApplicationSession::standAlone()
{
  return ::standAlone();
}
}
