#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include "pamclientauthenticator.hpp"

qiLogCategory("qimessaging.pamclientauthenticator");

namespace qi
{
const std::string PAMClientAuthenticator::PAM_USER_KEY = "pam_user";
const std::string PAMClientAuthenticator::PAM_PASSWORD_KEY = "pam_pass";

PAMClientAuthenticator::PAMClientAuthenticator(const std::string& user, const std::string& pass)
{
  _identity[PAM_USER_KEY] = AnyValue::from(user);
  _identity[PAM_PASSWORD_KEY] = AnyValue::from(pass);
}

PAMClientAuthenticator::~PAMClientAuthenticator()
{
}

CapabilityMap PAMClientAuthenticator::initialAuthData()
{
  return _identity;
}

CapabilityMap PAMClientAuthenticator::_processAuth(const CapabilityMap& c)
{
  qiLogDebug() << "Unexpected step in PAM authentication. Caps:";
  for (CapabilityMap::const_iterator it = c.begin(), end = c.end(); it != end; ++it)
    qiLogDebug() << it->first;
  throw std::runtime_error("Unexpected step in the PAM authentication process: are you using the proper authenticator?");
}

PAMClientAuthenticatorFactory::PAMClientAuthenticatorFactory(const std::string& user, const std::string& pass)
  : _user(user)
  , _pass(pass)
{
}

PAMClientAuthenticatorFactory::~PAMClientAuthenticatorFactory()
{
}

ClientAuthenticatorPtr PAMClientAuthenticatorFactory::newAuthenticator()
{
  return boost::make_shared<PAMClientAuthenticator>(_user, _pass);
}
}
