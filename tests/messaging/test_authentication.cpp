/*
**  Author(s):
**   - Thomas Fontenay <tfontenay@aldebaran.com>
**
** Copyright (c) 2014 Aldebaran Robotics. All rights reserved.
**
*/

#include <gtest/gtest.h>

#include <boost/scoped_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/filesystem.hpp>

#include <qi/session.hpp>
#include <qi/log.hpp>
#include <qi/os.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/application.hpp>
#include <testssl/testssl.hpp>
#include "src/messaging/authprovider_p.hpp"

qiLogCategory("TestAuthentication");

class TestAuthentication : public ::testing::Test
{
protected:
  void SetUp() override
  {
    qi::Session::Config sdConfig;
    sdConfig.serverSslConfig = test::ssl::serverConfig(test::ssl::server());

// Ignore use of deprecated Session constructor.
// Remove when the use of the constructor is removed.
KA_WARNING_PUSH()
KA_WARNING_DISABLE(4996, deprecated-declarations)
    sd_ = qi::SessionPtr(new qi::Session(true, std::move(sdConfig)));
    sd_->listenStandalone("tcps://127.0.0.1:0");

    client_ = qi::SessionPtr(new qi::Session(true));
KA_WARNING_POP()
  }

  void TearDown() override
  {
    sd_->close();
  }

  qi::SessionPtr sd_;
  qi::SessionPtr client_;
};

namespace {

  int service_func(int input)
  {
    return input + 44;
  }

  class UserPassProvider : public qi::AuthProvider {
  public:
    qi::CapabilityMap _processAuth(const qi::CapabilityMap& ad)
    {
      qi::CapabilityMap result;
      std::string remoteUser = ad.find(user_key) != ad.end() ? ad.find(user_key)->second.to<std::string>() : "";
      std::string remotePass = ad.find(pass_key) != ad.end() ? ad.find(pass_key)->second.to<std::string>() : "";

      if (remoteUser != user_ || remotePass != pass_)
        result[AuthProvider::State_Key] = qi::AnyValue::from(AuthProvider::State_Error);
      else
        result[AuthProvider::State_Key] = qi::AnyValue::from(AuthProvider::State_Done);
      return result;
    }
    static const std::string user_key;
    static const std::string pass_key;
    std::string user_;
    std::string pass_;
  };
  const std::string UserPassProvider::user_key = "user";
  const std::string UserPassProvider::pass_key = "pass";

  class UserPassProviderFactory : public qi::AuthProviderFactory {
  public:
    qi::AuthProviderPtr newProvider()
    {
      UserPassProvider* prov(new UserPassProvider);
      prov->user_ = expectedUser_;
      prov->pass_ = expectedPass_;
      return qi::AuthProviderPtr(prov);
    }
    std::string expectedUser_;
    std::string expectedPass_;
  };

  class UserPassAuthenticator : public qi::ClientAuthenticator {
  public:
    UserPassAuthenticator(const std::string& user, const std::string& pass) : _u(user), _p(pass) {}

    qi::CapabilityMap initialAuthData()
    {
      qi::CapabilityMap result;
      result[UserPassProvider::user_key] = qi::AnyValue::from(_u);
      result[UserPassProvider::pass_key] = qi::AnyValue::from(_p);
      return result;
    }
    qi::CapabilityMap _processAuth(const qi::CapabilityMap&)
    {
      return qi::CapabilityMap();
    }
  private:
    std::string _u;
    std::string _p;
  };
  class UserPassAuthenticatorFactory : public qi::ClientAuthenticatorFactory {
  public:
    qi::ClientAuthenticatorPtr newAuthenticator() {
      return boost::make_shared<UserPassAuthenticator>(user, pass);
    }
    std::string user;
    std::string pass;
  };


  class MultiStepProvider : public qi::AuthProvider {
  public:
    MultiStepProvider(int nstep = 2) { step = nstep; }
    qi::CapabilityMap _processAuth(const qi::CapabilityMap& /*ad*/)
    {
      qi::CapabilityMap result;

      if (--step)
        result[qi::AuthProvider::State_Key] = qi::AnyValue::from(AuthProvider::State_Cont);
      else
        result[qi::AuthProvider::State_Key] = qi::AnyValue::from(AuthProvider::State_Done);
      return result;
    }
    int step;
  };
  class MultiStepProviderFactory : public qi::AuthProviderFactory {
  public:
    virtual qi::AuthProviderPtr newProvider() { return qi::AuthProviderPtr(new MultiStepProvider); }
  };

  class MultiStepAuthenticator: public qi::ClientAuthenticator {
  public:
    MultiStepAuthenticator(int nstep = 2) { step = nstep - 1; }

    qi::CapabilityMap _processAuth(const qi::CapabilityMap&)
    {
      return qi::CapabilityMap();
    }
    int step;
  };
  class MultiStepAuthenticatorFactory: public qi::ClientAuthenticatorFactory {
  public:

    virtual qi::ClientAuthenticatorPtr newAuthenticator() { return qi::ClientAuthenticatorPtr(new MultiStepAuthenticator); }
  };

  class HarshProvider : public qi::AuthProvider {
  public:
    qi::CapabilityMap _processAuth(const qi::CapabilityMap& /*ad*/)
    {
       qi::CapabilityMap result;

       result[AuthProvider::State_Key] = qi::AnyValue::from(AuthProvider::State_Error);
       result[AuthProvider::Error_Reason_Key] = qi::AnyValue::from("Glory to Artzotska");
       return result;
    }
  };
  class HarshProviderFactory : public qi::AuthProviderFactory {
  public:
    qi::AuthProviderPtr newProvider() { return qi::AuthProviderPtr(new HarshProvider); }
  };

}

TEST_F(TestAuthentication, AuthErrorDisconnectsClient)
{
  sd_->setAuthProviderFactory(boost::make_shared<HarshProviderFactory>());

  qi::Future<void> conn1 = client_->connect(sd_->url());

  ASSERT_TRUE(conn1.hasError());
  ASSERT_FALSE(client_->isConnected());
}

TEST_F(TestAuthentication, UserPassTest)
{
  std::string user = "Bob_l_eponge";
  std::string pass = "much_password_many_authentication_wow";
  UserPassAuthenticatorFactory* clientFactory = new UserPassAuthenticatorFactory;
  clientFactory->user = user;
  clientFactory->pass = pass;
  UserPassProviderFactory* factory = new UserPassProviderFactory;
  factory->expectedUser_ = user;
  factory->expectedPass_ = pass;
  sd_->setAuthProviderFactory(qi::AuthProviderFactoryPtr(factory));
  client_->setClientAuthenticatorFactory(qi::ClientAuthenticatorFactoryPtr(clientFactory));

  qi::Future<void> connecting = client_->connect(sd_->url());
  ASSERT_FALSE(connecting.hasError()) << "Error was: " << connecting.error();
  ASSERT_TRUE(client_->isConnected());
}

TEST_F(TestAuthentication, UserPassFailTest)
{
  std::string user = "omelette_du_fromage";
  std::string pass = "dyel94";
  UserPassAuthenticatorFactory* clientFactory = new UserPassAuthenticatorFactory;
  clientFactory->user = user;
  clientFactory->pass = pass;
  UserPassProviderFactory* factory = new UserPassProviderFactory;
  factory->expectedUser_ = "user";
  factory->expectedPass_ = "pass";
  sd_->setAuthProviderFactory(qi::AuthProviderFactoryPtr(factory));
  client_->setClientAuthenticatorFactory(qi::ClientAuthenticatorFactoryPtr(clientFactory));

  auto connecting = client_->connect(sd_->url());
  EXPECT_EQ(qi::FutureState_FinishedWithError, connecting.waitFor(qi::MilliSeconds{500}));
  ASSERT_FALSE(client_->isConnected());
}

TEST_F(TestAuthentication, MultiStepAuthenticationTest)
{
  sd_->setAuthProviderFactory(qi::AuthProviderFactoryPtr(new MultiStepProviderFactory));
  client_->setClientAuthenticatorFactory(qi::ClientAuthenticatorFactoryPtr(new MultiStepAuthenticatorFactory));

  qi::Future<void> conn = client_->connect(sd_->url());
  ASSERT_FALSE(conn.hasError());
  ASSERT_TRUE(client_->isConnected());
}

TEST_F(TestAuthentication, ConnectionToServiceTest)
{
  std::string user = "Bob_l_eponge";
  std::string pass = "much_password_many_authentication_wow";
  boost::shared_ptr<UserPassAuthenticatorFactory> clientFactory = boost::make_shared<UserPassAuthenticatorFactory>();
  clientFactory->user = user;
  clientFactory->pass = pass;
  boost::shared_ptr<UserPassProviderFactory> factory = boost::make_shared<UserPassProviderFactory>();
  factory->expectedUser_ = user;
  factory->expectedPass_ = pass;
  sd_->setAuthProviderFactory(factory);
  client_->setClientAuthenticatorFactory(clientFactory);

  auto serviceHost = qi::makeSession();
  qi::DynamicObjectBuilder builder;
  builder.advertiseMethod<int (int)>("toto", &service_func);
  qi::AnyObject service = builder.object();

  serviceHost->setClientAuthenticatorFactory(clientFactory);
  serviceHost->setAuthProviderFactory(factory);
  serviceHost->connect(sd_->url());
  ASSERT_TRUE(serviceHost->isConnected());
  serviceHost->listen("tcp://localhost:0");
  serviceHost->registerService("toto", service);

  client_->connect(sd_->url());
  ASSERT_TRUE(client_->isConnected());

  qi::AnyObject myObj = client_->service("toto").value();
  ASSERT_EQ(myObj.call<int>("toto", 44), 88);
}
