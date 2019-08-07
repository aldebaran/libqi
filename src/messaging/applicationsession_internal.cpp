/*
**  Copyright (C) 2019 SoftBank Robotics Europe
**  See COPYING for the license
*/

#include "applicationsession_internal.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/program_options.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <qi/url.hpp>

qiLogCategory("qi.applicationsession");

namespace qi
{
namespace appsession_internal
{

const char* const qiUrlArgVar = "qi-url";
const char* const qiListenUrlArgVar = "qi-listen-url";
const char* const qiStandaloneArgVar = "qi-standalone";

extern const char* const qiUrlEnvVar = "QI_URL";
extern const char* const qiListenUrlEnvVar = "QI_LISTEN_URL";

using namespace boost::program_options;

ProgramOptions::ProgramOptions() = default;

ProgramOptions::ProgramOptions(const std::vector<std::string>& args)
{
  variables_map vm;

  // Priority for configuration values are: command line over environment over hardcoded default
  // value.
  auto parsedCli = command_line_parser(args).options(description()).allow_unregistered().run();
  store(parsedCli, vm);

  standalone = vm[qiStandaloneArgVar].as<bool>();
  if (!vm[qiListenUrlArgVar].empty())
    hasCliListenUrl = true;

  // Having both standalone and a connect URL as command line arguments is not acceptable.
  if (standalone && !vm[qiUrlArgVar].empty())
    throw std::runtime_error(std::string("You cannot specify both --") + qiStandaloneArgVar +
                             " and --" + qiUrlArgVar + " to connect.");

  auto parsedEnv =
    parse_environment(description(), [](const std::string& envVar) -> std::string {
      if (envVar == qiUrlEnvVar)
        return qiUrlArgVar;
      if (envVar == qiListenUrlEnvVar)
        return qiListenUrlArgVar;
      return {};
    });
  store(parsedEnv, vm);

  notify(vm);

  const auto maybeValue = [&](const std::string& name) -> boost::optional<std::string> {
    const auto slotIt = vm.find(name);
    if (slotIt == vm.end())
      return {};
    const auto& value = slotIt->second;
    return value.as<std::string>();
  };

  connectAddress = maybeValue(qiUrlArgVar);
  listenAddresses = maybeValue(qiListenUrlArgVar);

  unrecognizedArgs = collect_unrecognized(parsedCli.options, include_positional);
}

const options_description& ProgramOptions::description()
{
  static const auto desc = [] {
    static const auto listenUrlDesc =
      "The URL to listen to.\n"
      "It can be multiple URL separated by semicolons, in which case the application will try to "
      "listen to all of them.\n"
      "  Example: tcp://127.0.0.1:9555;tcp://:9999;127.0.0.1\n"
      "Missing information from incomplete URL will be defaulted with parts of the URL '" +
      SessionConfig::defaultListenUrl().str() +
      "'.\n"
      "If the default URL is tcps://0.0.0.0:9559 the previous list will become:\n"
      "  Example: tcp://127.0.0.1:9555;tcp://0.0.0.0:9999;tcps://127.0.0.1:9559";

    options_description desc("ApplicationSession options");
    desc.add_options()
      ( qiUrlArgVar,
        value<std::string>(),
        "The URL of the service directory to connect to."
      )
      ( qiListenUrlArgVar,
        value<std::string>(),
        listenUrlDesc.c_str()
      )
      ( qiStandaloneArgVar,
        bool_switch()->default_value(false),
        "Set the application as a standalone.\nA standalone application does not connect to a "
        "service directory but instead act as one."
      );
    return desc;
  }();
  return desc;
}

std::string urlVecToString(const UrlVector& urls, const std::string& sep)
{
  return boost::join(boost::adaptors::transform(urls, [](const Url& url) { return url.str(); }),
                     sep);
}

UrlVector stringToUrlVec(boost::optional<std::string> addresses, char sep)
{
  if (!addresses)
    return {};

  std::vector<std::string> addressList;
  boost::split(addressList, *addresses, boost::lambda::_1 == sep);
  QI_ASSERT_FALSE(addressList.empty());
  const auto url =
    boost::adaptors::transform(addressList, [](const std::string& addr) { return Url(addr); });
  return { url.begin(), url.end() };
}

namespace
{

Url prefillConnectUrl(boost::optional<Url> connectUrl)
{
  return specifyUrl(connectUrl.value_or(Url{}), SessionConfig::defaultConnectUrl());
}

UrlVector prefillListenUrls(UrlVector listenUrls)
{
  if (listenUrls.empty())
    listenUrls.push_back(SessionConfig::defaultListenUrl());
  else
  {
    for (auto& listenUrl : listenUrls)
    {
      if (!listenUrl.isValid())
        listenUrl = specifyUrl(listenUrl, SessionConfig::defaultListenUrl());
    }
  }
  return listenUrls;
}

}

Config reconfigureWithUrl(Config conf,
                          boost::optional<Url> newConnectUrl,
                          const UrlVector& newListenUrl)
{
  {
    auto connectUrl = prefillConnectUrl(conf.connectUrl());
    if (newConnectUrl)
    {
      qiLogVerbose() << "Connect URL specified: '" << *newConnectUrl
                     << "', now defaulting missing URL parts with " << connectUrl;
      connectUrl = specifyUrl(*newConnectUrl, connectUrl);
    }

    qiLogVerbose() << "Connect URL is now: " << connectUrl;
    conf.setConnectUrl(connectUrl);
  }

  {
    auto listenUrl = prefillListenUrls(conf.listenUrls());
    if (!newListenUrl.empty())
    {
      const auto newListenUrlStr = urlVecToString(newListenUrl, " ");
      const auto baseListenUrl = listenUrl.front();
      QI_ASSERT_TRUE(baseListenUrl.isValid());
      qiLogVerbose() << "Listen URLs specified: {" << newListenUrlStr
                     << "}, now defaulting missing URL parts with " << baseListenUrl;

      const auto specifyUrlWithBase = [&](const Url& url){ return specifyUrl(url, baseListenUrl); };
      const auto specifiedNewListenUrl =
        boost::adaptors::transform(newListenUrl, specifyUrlWithBase);
      listenUrl.assign(specifiedNewListenUrl.begin(), specifiedNewListenUrl.end());
    }

    qiLogVerbose() << "Listen URLs are now: " << urlVecToString(listenUrl, " ");
    conf.setListenUrls(listenUrl);
  }

  return conf;
}

Config reconfigureWithProgramOptions(Config conf, const ProgramOptions& progOpts)
{
  conf.setStandalone(conf.standalone() || progOpts.standalone);
  conf = reconfigureWithUrl(std::move(conf),
                            progOpts.connectAddress ?
                              boost::make_optional<Url>(*progOpts.connectAddress) :
                              boost::none,
                            stringToUrlVec(progOpts.listenAddresses));

  // The configuration must include a valid connect URL and one or more valid listen URL.
  const auto& connectUrl = conf.connectUrl();
  QI_IGNORE_UNUSED(connectUrl);
  QI_ASSERT_TRUE(connectUrl.is_initialized());
  QI_ASSERT_TRUE(connectUrl->isValid());

  const auto& listenUrls = conf.listenUrls();
  QI_IGNORE_UNUSED(listenUrls);
  QI_ASSERT_FALSE(listenUrls.empty());
  QI_ASSERT_TRUE(std::all_of(listenUrls.begin(), listenUrls.end(),
                             [](const Url& url) { return url.isValid(); }));

  return conf;
}

} // namespace appsession_internal
} // namespace qi
