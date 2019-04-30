/*
**  Copyright (C) 2019 SoftBank Robotics Europe
**  See COPYING for the license
*/

#ifndef QI_MESSAGING_APPSESSION_INTERNAL_HPP
#define QI_MESSAGING_APPSESSION_INTERNAL_HPP

#pragma once

#include <qi/applicationsession.hpp>

namespace qi
{

namespace appsession_internal
{

extern const char* const qiUrlEnvVar;
extern const char* const qiListenUrlEnvVar;

struct ProgramOptions
{
// Regular
  ProgramOptions();
  KA_GENERATE_FRIEND_REGULAR_OPS_4(ProgramOptions,
                                   connectAddress,
                                   listenAddresses,
                                   hasCliListenUrl,
                                   standalone)

// ProgramOptions:
  ProgramOptions(const std::vector<std::string>& args);

  static const boost::program_options::options_description& description();

  boost::optional<std::string> connectAddress;
  boost::optional<std::string> listenAddresses;
  bool hasCliListenUrl = false;
  bool standalone = false;
  std::vector<std::string> unrecognizedArgs;
};

/// @returns A string representation of all the URL joined with the given separator. If the list is
/// empty, returns an empty string.
std::string urlVecToString(const UrlVector& urls, const std::string& sep = ";");

/// @returns A list of URL constructed by splitting the addresses according to the separator. If
/// `addresses` is not set, returns an empty list. If `addresses` is set and is an empty string,
/// returns a list of one URL constructed from the empty string.
UrlVector stringToUrlVec(boost::optional<std::string> addresses, char sep = ';');

using Config = ApplicationSession::Config;

/// @details Creates a configuration with refined URL by ensuring it contains a valid connect URL
/// and at least one valid listen URL. If not empty, the URL given as parameters will be used in
/// priority after they are completed by the current URL of the configuration.
///
/// @post The returned configuration holds a valid connect URL and one or more valid listen URL.
Config reconfigureWithUrl(Config conf,
                          boost::optional<Url> newConnectUrl = {},
                          const UrlVector& newListenUrl = {});

/// Creates a configuration with the program options.
/// @post The returned configuration holds a valid connect URL and one or more valid listen URL.
Config reconfigureWithProgramOptions(Config conf, const ProgramOptions& progOpts);

}

}

#endif
