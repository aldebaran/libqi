/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <qi/assert.hpp>
#include <qi/log.hpp>
#include <qi/type/typeinterface.hpp>

#include "authprovider_p.hpp"

qiLogCategory("qimessaging.authprovider");

namespace qi {

  /* TODO: The benefit / purpose of the following prefixes is not clear. */
  /*       Further analysis required */
  const std::string AuthProvider::QiAuthPrefix     = "__qi_auth_";
  const std::string AuthProvider::UserAuthPrefix   = "auth_";
  const std::string AuthProvider::Error_Reason_Key = QiAuthPrefix + "err_reason";
  const std::string AuthProvider::State_Key        = QiAuthPrefix + "state";

  namespace auth_provider_private
  {
    static CapabilityMap extractAuthData(const CapabilityMap& cmap)
    {
      CapabilityMap authData;
      // Extract all capabilities related to authentication
      for (CapabilityMap::const_iterator it = cmap.begin(), end = cmap.end(); it != end; ++it)
      {
        const std::string& key = it->first;
        if (boost::algorithm::starts_with(key, AuthProvider::QiAuthPrefix))
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
        if (boost::algorithm::starts_with(it->first, AuthProvider::QiAuthPrefix))
          result[it->first] = it->second;
        else
          result[AuthProvider::UserAuthPrefix + it->first] = it->second;
      }
      return result;
    }
  } // auth_provider_private

  CapabilityMap AuthProvider::processAuth(const CapabilityMap &authData)
  {
    CapabilityMap result;

    result = auth_provider_private::extractAuthData(authData);
    result = _processAuth(result);
    QI_ASSERT(result.find(AuthProvider::State_Key) != result.end());
    return auth_provider_private::prepareAuthCaps(result);
  }

  AuthProviderPtr NullAuthProviderFactory::newProvider()
  {
    return boost::make_shared<NullAuthProvider>();
  }

  CapabilityMap NullAuthProvider::_processAuth(const CapabilityMap& /*authData*/)
  {
    CapabilityMap reply;
    reply[State_Key] = AnyValue::from<unsigned int>(State_Done);
    return reply;
  }

}
