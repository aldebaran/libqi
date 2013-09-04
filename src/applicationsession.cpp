/*
 * Copyright (c) 2013 Aldebaran Robotics. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the COPYING file.
 */

#include <boost/program_options.hpp>
#include <qi/trackable.hpp>
#include <qimessaging/applicationsession.hpp>

static void onDisconnected(const std::string& /*errorMessage*/)
{
  ::qi::Application::stop();
}

static std::string _address;
static std::string _listenAddress;
static void parseAddress()
{
  namespace po = boost::program_options;
  po::options_description desc("ApplicationSession options");

  desc.add_options()
      ("qi-url", po::value<std::string>(&_address), "The address of the service directory")
      ("qi-listen-url", po::value<std::string>(&_listenAddress), "The url to listen to");
  po::variables_map vm;
  po::parsed_options parsed = po::command_line_parser(qi::Application::arguments()).options(desc).allow_unregistered().run();
  po::store(parsed, vm);
  po::notify(vm);
  qi::Application::setArguments(po::collect_unrecognized(parsed.options, po::include_positional));

  {
    po::options_description descTmp;
    descTmp.add_options()
      ("help,h", "");
    po::variables_map vmTmp;
    po::store(po::command_line_parser(qi::Application::arguments()).options(descTmp).allow_unregistered().run(), vmTmp);
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
    ApplicationSessionPrivate(const Url& url, ApplicationSessionOptions opt);
    virtual ~ApplicationSessionPrivate();

    void connect();
  public:
    Url          _url;
    Session      _session;
    bool         _init;
    boost::mutex _mutex;
  };

  ApplicationSession::ApplicationSession(int&                      argc,
                                         char**&                   argv,
                                         ApplicationSessionOptions opt,
                                         const Url&                url)
    : Application(::addParseOptions(argc), argv)
  {
    _p = new ApplicationSessionPrivate(url, opt);
  }

  ApplicationSession::ApplicationSession(const std::string&        name,
                                         int&                      argc,
                                         char**&                   argv,
                                         ApplicationSessionOptions opt,
                                         const Url&                url)
    : Application(name, ::addParseOptions(argc), argv)
  {
    _p = new ApplicationSessionPrivate(url, opt);
  }


  ApplicationSession::~ApplicationSession()
  {
    delete _p;
    _p = 0;
  }

  Session& ApplicationSession::session()
  {
    return _p->_session;
  }

  const Url& ApplicationSession::url()
  {
    return _p->_url;
  }

  const Url& ApplicationSession::listenUrl()
  {
    return _listenAddress;
  }

  void ApplicationSession::start()
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
    start();
    Application::run();
  }

  ApplicationSessionPrivate::ApplicationSessionPrivate(const Url& url, ApplicationSessionOptions opt)
    : Trackable<ApplicationSessionPrivate>(this)
    , _init(false)
  {
    if (!(opt & ApplicationSession_NoAutoExit))
    {
      _session.disconnected.connect(&::onDisconnected);
    }

    _url = _address.empty() ? url : Url(_address, "tcp", 9559);
  }

  ApplicationSessionPrivate::~ApplicationSessionPrivate()
  {
    destroy();
  }

  void ApplicationSessionPrivate::connect()
  {
    _session.connect(_url);

    if (!_listenAddress.empty())
    {
      Url listenUrl(_listenAddress, "tcp", 0);
      _session.listen(listenUrl);
    }
  }
}
