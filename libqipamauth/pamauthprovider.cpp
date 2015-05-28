#include <security/pam_appl.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <qi/log.hpp>

#include "pamclientauthenticator.hpp"
#include "pamauthprovider.hpp"

qiLogCategory("qimessaging.pamauthprovider");

namespace qi
{

static int pam_conv_cb(int nmsg, const pam_message** msg, pam_response** resp, void* data)
{
  int status = PAM_SUCCESS;
  std::string* pass = static_cast<std::string*>(data);
  if (nmsg <= 0)
    return PAM_CONV_ERR;

  pam_response* responses = static_cast<pam_response*>(calloc(nmsg, sizeof(pam_response)));
  for (int i = 0; i < nmsg; ++i)
  {
    qiLogDebug() << "Message " << i << ": " << msg[i]->msg;
    assert(std::strncmp(msg[i]->msg, "Password:", 9) == 0);
    responses[i].resp_retcode = 0;

    switch (msg[i]->msg_style)
    {
    case PAM_PROMPT_ECHO_OFF:
      responses[i].resp = qi::os::strdup(pass->c_str());
      break;
    case PAM_ERROR_MSG:
    case PAM_PROMPT_ECHO_ON:
    case PAM_TEXT_INFO:
      status = PAM_CONV_ERR;
      break;
    }
  }
  if (status != PAM_SUCCESS)
  {
    for (int i = 0; i < nmsg; ++i)
    {
      char* r = responses[i].resp;
      if (r)
      {
        std::memset(r, 0, std::strlen(r));
        std::free(r);
        responses[i].resp = NULL;
      }
    }
    std::free(responses);
    responses = NULL;
  }
  *resp = responses;
  return status;
}

static CapabilityMap& pamErrorCleanup(CapabilityMap& resp, pam_handle_t* handle, int pamStatus)
{
  qiLogDebug() << "PAM authentication failed: " << pam_strerror(handle, pamStatus);

  pam_end(handle, pamStatus);
  resp[AuthProvider::State_Key] = AnyValue::from(AuthProvider::State_Error);
  resp[AuthProvider::Error_Reason_Key] = AnyValue::from("PAM Authentication failed.");
  return resp;
}

CapabilityMap PAMAuthProvider::_processAuth(const CapabilityMap& authData)
{
  CapabilityMap reply;
  CapabilityMap::const_iterator userIt = authData.find(PAMClientAuthenticator::PAM_USER_KEY),
      passIt = authData.find(PAMClientAuthenticator::PAM_PASSWORD_KEY);

  if (userIt == authData.end() || passIt == authData.end())
  {
    reply[AuthProvider::State_Key] = AnyValue::from(AuthProvider::State_Error);
    reply[AuthProvider::Error_Reason_Key] = AnyValue::from("PAM Authenticator: provide user name in '" +
                                                           PAMClientAuthenticator::PAM_USER_KEY +
                                                           "' and password in '" +
                                                           PAMClientAuthenticator::PAM_PASSWORD_KEY + "'");
    return reply;
  }

  std::string user = userIt->second.to<std::string>();
  std::string pass = passIt->second.to<std::string>();
  pam_handle_t* pamHandle = NULL;
  pam_conv conversation = {&pam_conv_cb, &pass};
  int pamStatus = PAM_SUCCESS;

  if (pam_start("login", user.c_str(), &conversation, &pamHandle) != PAM_SUCCESS)
  {
    reply[AuthProvider::State_Key] = AnyValue::from(AuthProvider::State_Error);
    return reply;
  }

  if ((pamStatus = pam_authenticate(pamHandle, 0)) != PAM_SUCCESS ||
      (pamStatus = pam_acct_mgmt(pamHandle, 0)) != PAM_SUCCESS)
    return pamErrorCleanup(reply, pamHandle, pamStatus);

  pam_end(pamHandle, pamStatus);
  reply[AuthProvider::State_Key] = AnyValue::from(AuthProvider::State_Done);
  return reply;
}

AuthProviderPtr PAMAuthProviderFactory::newProvider()
{
  return boost::make_shared<PAMAuthProvider>();
}
}
