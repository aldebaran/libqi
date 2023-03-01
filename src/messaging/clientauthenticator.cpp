/*
**  Copyright (C) 2014 Aldebaran Robotics
**  See COPYING for the license
*/

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <qi/log.hpp>
#include <qi/messaging/authprovider.hpp>

#include "authprovider_p.hpp"
#include "clientauthenticator_p.hpp"

qiLogCategory("qimessaging.clientauthenticator");

namespace qi
{
  namespace client_authenticator_private
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
  } // client_authenticator_private

  CapabilityMap ClientAuthenticator::processAuth(const CapabilityMap &authData)
  {
    CapabilityMap result;

    result = client_authenticator_private::extractAuthData(authData);
    result = _processAuth(result);
    return client_authenticator_private::prepareAuthCaps(result);
  }


  CapabilityMap ClientAuthenticator::initialAuthData()
  {
    return CapabilityMap();
  }

  ClientAuthenticatorPtr NullClientAuthenticatorFactory::newAuthenticator()
  {
    return boost::make_shared<NullClientAuthenticator>();
  }

  CapabilityMap NullClientAuthenticator::_processAuth(const CapabilityMap& /*authData*/)
  {
    return CapabilityMap();
  }

}
