/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <qi/log.hpp>
#include <qi/type/typeinterface.hpp>

#include "authprovider_p.hpp"

qiLogCategory("qimessaging.authprovider");

namespace qi {

  const std::string QiAuthPrefix = "__qi_auth_";
  const std::string AuthProvider::UserAuthPrefix = "auth_";
  const std::string AuthProvider::Error_Reason_Key = QiAuthPrefix + "err_reason";
  const std::string AuthProvider::State_Key = QiAuthPrefix + "state";

  static CapabilityMap extractAuthData(const CapabilityMap& cmap)
  {
    CapabilityMap authData;
    // Extract all capabilities related to authentication
    for (CapabilityMap::const_iterator it = cmap.begin(), end = cmap.end(); it != end; ++it)
    {
      const std::string& key = it->first;
      if (boost::algorithm::starts_with(key, QiAuthPrefix))
        authData[key] = it->second;
      else if (boost::algorithm::starts_with(key, AuthProvider::UserAuthPrefix))
        authData[key.substr(AuthProvider::UserAuthPrefix.length(), std::string::npos)] = it->second;
    }
    return authData;
  }

  static CapabilityMap prepareAuthCaps(const CapabilityMap& data)
  {
    CapabilityMap result;

    for (CapabilityMap::const_iterator it = data.begin(), end = data.end(); it != end; ++it)
    {
      if (boost::algorithm::starts_with(it->first, QiAuthPrefix))
        result[it->first] = it->second;
      else
        result[AuthProvider::UserAuthPrefix + it->first] = it->second;
    }
    return result;
  }

  CapabilityMap AuthProvider::processAuth(const CapabilityMap &authData)
  {
    CapabilityMap result;

    result = extractAuthData(authData);
    result = _processAuth(result);
    assert(result.find(AuthProvider::State_Key) != result.end());
    return prepareAuthCaps(result);
  }

  AuthProviderPtr NullAuthProviderFactory::newProvider()
  {
    return boost::make_shared<NullAuthProvider>();
  }

  CapabilityMap NullAuthProvider::_processAuth(const CapabilityMap &authData)
  {
    CapabilityMap reply;

    reply[State_Key] = AnyValue::from(State_Done);
    return reply;
  }

}
