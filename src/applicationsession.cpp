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

namespace qi
{
  namespace po = boost::program_options;
  class ApplicationSessionPrivate : public Trackable<ApplicationSessionPrivate>
  {
  public:
    ApplicationSessionPrivate(const Url& url, ApplicationSessionOptions opt);
    virtual ~ApplicationSessionPrivate();

    Url  retrieveUrl(const Url& url) const;
  public:
    Session                   _session;
  };

  static ApplicationSessionPrivate* _p = 0;

  ApplicationSession::ApplicationSession(int&                      argc,
                                         char**&                   argv,
                                         ApplicationSessionOptions opt,
                                         const Url&                url)
    : Application(argc, argv)
  {
    if (!_p)
    {
      _p = new ApplicationSessionPrivate(url, opt);
    }
  }

  ApplicationSession::ApplicationSession(const std::string&        name,
                                         int&                      argc,
                                         char**&                   argv,
                                         ApplicationSessionOptions opt,
                                         const Url&                url)
    : Application(name, argc, argv)
  {
    if (!_p)
    {
      _p = new ApplicationSessionPrivate(url, opt);
    }
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

  ApplicationSessionPrivate::ApplicationSessionPrivate(const Url& url, ApplicationSessionOptions opt)
    : Trackable<ApplicationSessionPrivate>(this)
  {
    if (!(opt & ApplicationSession_NoAutoExit))
    {
      _session.disconnected.connect(&::onDisconnected);
    }

    Url urlTmp = retrieveUrl(url);

    if (!(opt & ApplicationSession_NoAutoConnect))
    {
      // The connection is synchronous, therefore a wait is expected here
      _session.connect(urlTmp);
    }
  }

  ApplicationSessionPrivate::~ApplicationSessionPrivate()
  {
    destroy();
  }

  Url ApplicationSessionPrivate::retrieveUrl(const Url& url) const
  {
    po::options_description options;

    options.add_options()
        ("qi-url", po::value<std::string>()->default_value(url.str()), "The address of the service directory");
    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(Application::arguments()).options(options).allow_unregistered().run();
    po::store(parsed, vm);
    po::notify(vm);

    Application::setArguments(po::collect_unrecognized(parsed.options, po::include_positional));

    // can't fail since qi-url's option has a default value
    return vm["qi-url"].as<std::string>();
  }
}
