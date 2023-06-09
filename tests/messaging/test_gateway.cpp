/*
 ** Author(s):
 **  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
 **
 ** Copyright (C) 2010, 2012 Aldebaran Robotics
 */

#include <string>
#include <random>
#include <fstream>

#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

#include <qi/anyobject.hpp>
#include <qi/application.hpp>
#include <qi/session.hpp>
#include <qi/future.hpp>
#include <qi/messaging/gateway.hpp>
#include <qi/type/dynamicobjectbuilder.hpp>
#include <qi/log.hpp>
#include <qi/testutils/testutils.hpp>

#include <testssl/testssl.hpp>

qiLogCategory("TestGateway");

namespace qi
{

  template<typename T>
  std::ostream& operator<<(std::ostream& os, const qi::Object<T>& obj)
  {
    if (obj.isValid())
      os << obj.uid() << "\n";
    else
      os << "<INVALID OBJECT>\n";
    return os;
  }

}

namespace
{

  using qi::SessionPtr;


  struct TestGatewayWithInvalidConnectUrl : testing::TestWithParam<qi::Url>{};

  INSTANTIATE_TEST_SUITE_P(
    InvalidUrls,
    TestGatewayWithInvalidConnectUrl,
    testing::Values(
      qi::Url("localhost:9559"),
      qi::Url("tcp://:9559"),
      qi::Url("tcp://localhost")));

  TEST_P(TestGatewayWithInvalidConnectUrl, FinishesWithError)
  {
    qi::Gateway::Config cfg;
    cfg.serviceDirectoryUrl = GetParam();
    EXPECT_TRUE(test::finishesWithError(qi::Gateway::create(cfg)));
  }

  namespace fs = boost::filesystem;

  struct TestGatewayConfig : ::testing::Test
  {
    struct RemoveFile
    {
      void operator()(const fs::path& p) const
      {
        fs::remove(p); // Does nothing if there is no path at `p`.
      }
    };

    ka::scoped_t<fs::path, RemoveFile> certChain = ka::scoped(writeTmpFile(test::ssl::getServerA1CertsChain()), RemoveFile{});
    ka::scoped_t<fs::path, RemoveFile> privateKey = ka::scoped(writeTmpFile(test::ssl::getServerA1Key()), RemoveFile{});
    ka::scoped_t<fs::path, RemoveFile> trustedCert = ka::scoped(writeTmpFile(test::ssl::getClientA1CertsChain()), RemoveFile{});

    static fs::path writeTmpFile(const std::string& content)
    {
      const auto filename = fs::temp_directory_path() / fs::unique_path();
      std::ofstream ofs(filename.string());
      EXPECT_TRUE(ofs);
      ofs << content;
      EXPECT_TRUE(ofs);
      return filename;
    }
  };

  namespace
  {
    // Relations to test equivalence of gateway configs and associated types.

    using namespace ::testing;

    struct EnrichFailure
    {
      std::string entityName;
      AssertionResult operator()(AssertionResult res) const
      {
        return AssertionFailure() << entityName << " not equal. Detail: " << res.message();
      }
    };

    // Convertible-to-bool T
    template<typename T>
    const char* emptyStr(const T& t)
    {
      return t ? "non-empty" : "empty";
    };

    struct EqAssert
    {
      // (Regular && OStreamable) T
      template<typename T>
      AssertionResult operator()(const T& a, const T& b) const
      {
        return a == b
          ? AssertionSuccess()
          : AssertionFailure() << "Values not equal. Detail: " << a << " != " << b;
      }
    };

    // concept RelationAssert(R, T):
    //   (): T × T -> AssertionResult

    struct EqOptional
    {
      // RelationAssert(T) EqT
      template<typename T, typename EqT = EqAssert>
      AssertionResult operator()(const boost::optional<T>& a, const boost::optional<T>& b, EqT eqT = {}) const
      {
        if (a.has_value() != b.has_value())
          return AssertionFailure() << "Optionals not equal. Detail: " << emptyStr(a) << " != " << emptyStr(b);
        if (! a.has_value())
          return AssertionSuccess(); // Both are empty.
        auto res = eqT(a.value(), b.value());
        if (! res)
          return EnrichFailure{"Optionals"}(res);
        return AssertionSuccess();
      }
    };

    struct EqCertificate
    {
      AssertionResult operator()(const qi::ssl::Certificate& a, const qi::ssl::Certificate& b) const
      {
        if (a.cmp(b) != 0)
          return AssertionFailure() << "Certificates not equal: " << &a << " != " << &b;
        return AssertionSuccess();
      }
    };

    struct EqVector
    {
      // RelationAssert(T) EqT
      template<typename T, typename EqT>
      AssertionResult operator()(const std::vector<T>& a, const std::vector<T>& b, EqT eqT) const
      {
        if (a.size() != b.size())
          return AssertionFailure() << "Vectors have different sizes: " << a.size() << " != " << b.size();
        auto res = AssertionSuccess();
        std::equal(a.begin(), a.end(), b.begin(), b.end(), [&res, eqT](const T& x, const T& y) -> bool {
          res = eqT(x, y);
          return res;
        });
        if (!res)
          return EnrichFailure{"Vectors"}(res);
        return AssertionSuccess();
      }
    };

    struct EqPKey
    {
      AssertionResult operator()(const qi::ssl::PKey& a, const qi::ssl::PKey& b) const
      {
        // Beware: `0` does not mean "equal", contrary to comparison of certificates.
        switch (a.eq(b))
        {
        case 0:
          return AssertionFailure() << "PKey not equal: " << &a << " != " << &b;
        case 1:
          return AssertionSuccess();
        default:
          throw std::invalid_argument("PKeys cannot be compared.");
        }
      }
    };

// If the predicate doesn't hold of given arguments, returns an assertion
// failure modified through the `enrich` transformation, which must be in
// scope. For examples, see tests below.
#define QI_HOLD_OR_RETURN_ENRICHED_FAILURE(PREDICATE, ...) \
{                                                          \
  auto res = PREDICATE(__VA_ARGS__);                       \
  if (! res) return enrich(ka::mv(res));                   \
}

    struct EqCertChainWithPrivateKey
    {
      AssertionResult operator()(const qi::ssl::CertChainWithPrivateKey& a, const qi::ssl::CertChainWithPrivateKey& b) const
      {
        auto enrich = EnrichFailure{"PKeys"};
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(EqVector{}, a.certificateChain, b.certificateChain, EqCertificate{});
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(EqPKey{}, a.privateKey, b.privateKey);
        return AssertionSuccess();
      }
    };

    struct PartialEqFunction
    {
      template<typename F>
      AssertionResult operator()(const std::function<F>& a, const std::function<F>& b) const
      {
        auto aValid = static_cast<bool>(a);
        auto bValid = static_cast<bool>(b);
        if (aValid != bValid)
          return AssertionFailure() << "Functions not equal. Detail: " << emptyStr(a) << " != " << emptyStr(b);
        if (!aValid)
          return AssertionSuccess(); // Both are non-valid.
        throw std::domain_error("Cannot compare non-empty std::functions.");
      }
    };

    // Contrary to `PartialEqFunction`, this relation is total and approximates
    // extensional equality through an input sample.
    //
    // (Regular && OStreamable) A
    template<typename A>
    struct EquivFunction
    {
      std::vector<A> inputSample;

      // (A -> _) F
      template<typename F>
      AssertionResult operator()(const std::function<F>& f, const std::function<F>& g) const
      {
        auto aValid = static_cast<bool>(f);
        auto bValid = static_cast<bool>(g);
        if (aValid != bValid)
          return AssertionFailure() << "Functions not equal. Detail: " << emptyStr(f) << " != " << emptyStr(g);
        if (!aValid)
          return AssertionSuccess(); // Both are non-valid.
        // Both are valid.
        for (const auto& a: inputSample)
        {
          if (f(a) != g(a)) return AssertionFailure() << "Functions not equal. Detail: "
            "f(" << a << ") = " << f(a) << " != " << g(a) << " = g(" << a << ")";
        }
        return AssertionSuccess();
      }
    };

    struct EqConfigBase
    {
      AssertionResult operator()(const qi::ssl::ConfigBase& a, const qi::ssl::ConfigBase& b) const
      {
        auto enrich = EnrichFailure{"ConfigBases"};
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(EqOptional{}, a.certWithPrivKey, b.certWithPrivKey, EqCertChainWithPrivateKey{});
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(EqVector{}, a.trustStore, b.trustStore, EqCertificate{});
        if (a.verifyPartialChain != b.verifyPartialChain)
        {
          return enrich(AssertionFailure() << "Verify partial chain not equal: "
            << a.verifyPartialChain << " != " << b.verifyPartialChain);
        }
        return AssertionSuccess();
      }
    };

    // (qi::ssl::ClientConfig || qi::ssl::ServerConfig) Config
    template<typename Config>
    struct EqConfig
    {
      AssertionResult operator()(const Config& a, const Config& b) const
      {
        auto enrich = EnrichFailure{entityName(a)};
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(EqConfigBase{}, a, b);
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(PartialEqFunction{}, a.verifyCallback, b.verifyCallback);
        return AssertionSuccess();
      }
      static std::string entityName(const qi::ssl::ClientConfig&)
      {
        return "ClientConfigs";
      }
      static std::string entityName(const qi::ssl::ServerConfig&)
      {
        return "ServerConfigs";
      }
    };

    using EqClientConfig = EqConfig<qi::ssl::ClientConfig>;
    using EqServerConfig = EqConfig<qi::ssl::ServerConfig>;

    struct EqGatewayConfig
    {
      EquivFunction<std::string> equivFilter;

      AssertionResult operator()(const qi::Gateway::Config& a, const qi::Gateway::Config& b)
      {
        auto enrich = [](AssertionResult res) {
          return AssertionFailure() << "Configs not equal. Detail: " << res.message();
        };

        if (a.serviceDirectoryUrl != b.serviceDirectoryUrl)
          return enrich(AssertionFailure() << "URL not equal: " << a.serviceDirectoryUrl << " != " << b.serviceDirectoryUrl);
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(EqVector{}, a.listenUrls, b.listenUrls, EqAssert{});
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(EqOptional{}, a.authProviderFactory, b.authProviderFactory);
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(EqOptional{}, a.clientAuthenticatorFactory, b.clientAuthenticatorFactory);
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(equivFilter, a.filterService.f, b.filterService.f);
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(EqClientConfig{}, a.clientSslConfig, b.clientSslConfig);
        QI_HOLD_OR_RETURN_ENRICHED_FAILURE(EqServerConfig{}, a.serverSslConfig, b.serverSslConfig);
        return AssertionSuccess();
      }
    };

#undef QI_HOLD_OR_RETURN_ENRICHED_FAILURE

    qi::ssl::ServerConfig makeTcpsServerConfig(fs::path certChainPath, fs::path privateKeyPath)
    {
      auto optKey = qi::ssl::PKey::privateFromPemFile(privateKeyPath);
      if (! optKey.has_value()) throw std::runtime_error("Cannot read key from " + privateKeyPath.string());

      auto config = qi::ssl::ServerConfig{};
      config.certWithPrivKey = qi::ssl::CertChainWithPrivateKey{
        qi::ssl::Certificate::chainFromPemFile(ka::mv(certChainPath)),
        *optKey
      };
      config.verifyPartialChain = false;
      return config;
    }

    qi::ssl::ServerConfig makeTcpsmServerConfig(fs::path certChainPath, fs::path privateKeyPath, fs::path trustedCertPath)
    {
      auto config = makeTcpsServerConfig(ka::mv(certChainPath), ka::mv(privateKeyPath));
      auto optTrust = qi::ssl::Certificate::fromPemFile(trustedCertPath);
      if (! optTrust.has_value()) throw std::runtime_error("Cannot read certificate from " + trustedCertPath.string());
      config.trustStore = { optTrust.value() };
      config.verifyPartialChain = true;
      return config;
    }

    struct DummyAuthProviderFactory : qi::AuthProviderFactory
    {
      qi::AuthProviderPtr newProvider()
      {
        return nullptr;
      }
    };

    qi::AuthProviderFactoryPtr dummyAuthProviderFactory()
    {
      static auto p = boost::make_shared<DummyAuthProviderFactory>();
      return p;
    }

    EquivFunction<std::string> equivFilterABC()
    {
      return { {"serviceA", "serviceB", "serviceC"} };
    }

    qi::Gateway::FilterService filterB()
    {
      return qi::Gateway::FilterService{ [](boost::string_ref s) -> bool {
        return s == "serviceB";
      }};
    }
  } // namespace

  TEST_F(TestGatewayConfig, TcpOk)
  {
    using Config = qi::Gateway::Config;
    auto sdUrl = qi::Url("tcp://127.0.0.1:12345");
    auto listeningUrl = qi::Url("tcp://127.0.0.1:57890");
    ASSERT_TRUE(
      EqGatewayConfig{equivFilterABC()}(
        Config{
          sdUrl, { listeningUrl }, dummyAuthProviderFactory(), boost::none, filterB(),
          qi::ssl::ClientConfig{},
          qi::ssl::ServerConfig{}
        },
        Config::createFromListeningProtocol(
          sdUrl, listeningUrl, filterB(), ka::unit, dummyAuthProviderFactory()
        )
      )
    );
  }

  TEST_F(TestGatewayConfig, TcpsOk)
  {
    using Config = qi::Gateway::Config;
    using TlsFilepaths = qi::Gateway::TlsFilepaths;
    const auto sdUrl = qi::Url("tcp://127.0.0.1:12345");
    const auto listeningUrl = qi::Url("tcps://127.0.0.1:57890");
    ASSERT_TRUE(
      EqGatewayConfig{equivFilterABC()}(
        Config{
          sdUrl, { listeningUrl }, dummyAuthProviderFactory(), boost::none, filterB(),
          qi::ssl::ClientConfig{},
          makeTcpsServerConfig(certChain.value, privateKey.value)
        },
        Config::createFromListeningProtocol(
          sdUrl, listeningUrl, filterB(),
          TlsFilepaths{certChain.value, privateKey.value},
          dummyAuthProviderFactory()
        )
      )
    );
  }

  TEST_F(TestGatewayConfig, TcpsmOk)
  {
    using Config = qi::Gateway::Config;
    using MTlsFilepaths = qi::Gateway::MTlsFilepaths;
    const auto sdUrl = qi::Url("tcp://127.0.0.1:12345");
    const auto listeningUrl = qi::Url("tcpsm://127.0.0.1:57890");
    ASSERT_TRUE(
      EqGatewayConfig{equivFilterABC()}(
        Config{
          sdUrl, { listeningUrl }, dummyAuthProviderFactory(), boost::none, filterB(),
          qi::ssl::ClientConfig{},
          makeTcpsmServerConfig(certChain.value, privateKey.value, trustedCert.value)
        },
        Config::createFromListeningProtocol(
          sdUrl, listeningUrl, filterB(),
          MTlsFilepaths{certChain.value, privateKey.value, trustedCert.value},
          dummyAuthProviderFactory()
        )
      )
    );
  }

  TEST_F(TestGatewayConfig, ThrowsOnWrongFilepathType)
  {
    auto tlsFilepaths = qi::Gateway::TlsFilepaths{certChain.value, privateKey.value};
    auto mTlsFilepaths = qi::Gateway::MTlsFilepaths{certChain.value, privateKey.value, trustedCert.value};
    auto createConfig = [](auto listenUrl, auto filepaths) {
      return qi::Gateway::Config::createFromListeningProtocol(
        qi::Url("tcp://127.0.0.1:12345"), ka::mv(listenUrl),
        filterB(), ka::mv(filepaths), dummyAuthProviderFactory()
      );
    };
    ASSERT_NO_THROW(createConfig("tcp://127.0.0.1:57890", ka::unit));
    ASSERT_THROW(createConfig("tcp://127.0.0.1:57890", tlsFilepaths), std::exception);
    ASSERT_THROW(createConfig("tcp://127.0.0.1:57890", mTlsFilepaths), std::exception);

    ASSERT_THROW(createConfig("tcps://127.0.0.1:57890", ka::unit), std::exception);
    ASSERT_NO_THROW(createConfig("tcps://127.0.0.1:57890", tlsFilepaths));
    ASSERT_THROW(createConfig("tcps://127.0.0.1:57890", mTlsFilepaths), std::exception);

    ASSERT_THROW(createConfig("tcpsm://127.0.0.1:57890", ka::unit), std::exception);
    ASSERT_THROW(createConfig("tcpsm://127.0.0.1:57890", tlsFilepaths), std::exception);
    ASSERT_NO_THROW(createConfig("tcpsm://127.0.0.1:57890", mTlsFilepaths));
  }

  TEST_F(TestGatewayConfig, ThrowsOnNonExistingFiles)
  {
    using TlsFilepaths = qi::Gateway::TlsFilepaths;
    using MTlsFilepaths = qi::Gateway::MTlsFilepaths;
    auto createConfig = [](auto listenUrl, auto filepaths) {
      return qi::Gateway::Config::createFromListeningProtocol(
        qi::Url("tcp://127.0.0.1:12345"), ka::mv(listenUrl),
        filterB(), ka::mv(filepaths), dummyAuthProviderFactory()
      );
    };
    const auto nonExistent = fs::path("/file/that/does/not/exist/hopefully");
    const auto tcpsUrl = qi::Url{"tcps://127.0.0.1:57890"};
    ASSERT_THROW(createConfig(tcpsUrl, TlsFilepaths{nonExistent,     privateKey.value}), std::exception);
    ASSERT_THROW(createConfig(tcpsUrl, TlsFilepaths{certChain.value, nonExistent}),      std::exception);
    ASSERT_THROW(createConfig(tcpsUrl, TlsFilepaths{nonExistent,     nonExistent}),      std::exception);

    const auto tcpsmUrl = qi::Url{"tcpsm://127.0.0.1:57890"};
    ASSERT_THROW(createConfig(tcpsmUrl, MTlsFilepaths{nonExistent,     privateKey.value, trustedCert.value}), std::exception);
    ASSERT_THROW(createConfig(tcpsmUrl, MTlsFilepaths{certChain.value, nonExistent,      trustedCert.value}), std::exception);
    ASSERT_THROW(createConfig(tcpsmUrl, MTlsFilepaths{nonExistent,     nonExistent,      trustedCert.value}), std::exception);
    ASSERT_THROW(createConfig(tcpsmUrl, MTlsFilepaths{certChain.value, privateKey.value, nonExistent}),       std::exception);
    ASSERT_THROW(createConfig(tcpsmUrl, MTlsFilepaths{nonExistent,     privateKey.value, nonExistent}),       std::exception);
    ASSERT_THROW(createConfig(tcpsmUrl, MTlsFilepaths{certChain.value, nonExistent,      nonExistent}),       std::exception);
    ASSERT_THROW(createConfig(tcpsmUrl, MTlsFilepaths{nonExistent,     nonExistent,      nonExistent}),       std::exception);
  }

  struct TestGatewayWithInvalidListenUrl : testing::TestWithParam<qi::Url>{};

  INSTANTIATE_TEST_SUITE_P(
    InvalidUrls,
    TestGatewayWithInvalidListenUrl,
    testing::Values(
      qi::Url("localhost:0"),
      qi::Url("tcp://:0"),
      qi::Url("tcp://localhost")));

  TEST_P(TestGatewayWithInvalidListenUrl, FinishesWithError)
  {
    qi::Gateway::Config cfg;
    cfg.serviceDirectoryUrl = "tcp://localhost:9559";
    // Even with a good listen URL, it still throws because of the bad one.
    cfg.listenUrls = { "tcp://localhost:0", GetParam() };
    EXPECT_TRUE(test::finishesWithError(qi::Gateway::create(cfg)));
  }

  class TestGateway : public ::testing::Test
  {
  public:
    TestGateway()
      : randEngine{ [] {
        std::random_device rd;
        std::seed_seq seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
        return std::default_random_engine{ seq };
      }() }
      , sd_{ qi::makeSession() }
    {}

    void SetUp()
    {
      sd_->listenStandalone("tcp://127.0.0.1:0");

      qi::Gateway::Config cfg;
      cfg.serviceDirectoryUrl = sd_->url();
      cfg.listenUrls = { "tcp://127.0.0.1:0" };
      const auto gwFut = qi::Gateway::create(std::move(cfg));
      ASSERT_TRUE(test::finishesWithValue(gwFut));
      gw_ = gwFut.value();
    }

    int randomValue()
    {
      return intDistrib(randEngine);
    }

    qi::SessionPtr connectClientToSd(qi::Session::Config cfg = {});
    qi::SessionPtr connectClientToGw(qi::Session::Config cfg = {});

    std::default_random_engine randEngine;
    std::uniform_int_distribution<int> intDistrib;
    qi::GatewayPtr gw_;
    qi::SessionPtr sd_;
  };

  int echoValue(int value)
  {
    qiLogInfo() << "echovalue: " << value;
    return value;
  }

  static qi::AnyObject makeBaseService();
  qi::AnyObject getObject()
  {
    return makeBaseService();
  }

  static qi::AnyObject makeBaseService()
  {
    qi::DynamicObjectBuilder ob;
    ob.advertiseMethod<int (int)>("echoValue", &echoValue);
    ob.advertiseMethod<qi::AnyObject (void)>("getObject", &getObject);
    ob.advertiseSignal<int>("echoSignal");
    ob.advertiseSignal<int>("echoSignal2");
    return ob.object();
  }

  qi::SessionPtr TestGateway::connectClientToSd(qi::Session::Config cfg)
  {
    cfg.connectUrl = sd_->url();
    qi::SessionPtr session = qi::makeSession(std::move(cfg));
    session->connect();
    return session;
  }

  qi::SessionPtr TestGateway::connectClientToGw(qi::Session::Config cfg)
  {
    cfg.connectUrl = gw_->endpoints().at(0);
    qi::SessionPtr session = qi::makeSession(std::move(cfg));
    session->connect();
    return session;
  }

  struct Callsync
  {
    Callsync(qi::Promise<int> prom, int expectedValue, int expectedCalls = 1, bool* hasOverflowed = NULL)
      : prom_(prom), exVal_(expectedValue), remainingCalls_(expectedCalls), ov_(hasOverflowed)
    {}

    Callsync(const Callsync& cs)
      : prom_(cs.prom_), exVal_(cs.exVal_), remainingCalls_(cs.remainingCalls_), ov_(cs.ov_), moutecks_()
    {}

    void operator()(int value)
    {
      boost::mutex::scoped_lock lock(moutecks_);
      qiLogInfo() << "Called:" << value;
      if (value != exVal_)
      {
        std::stringstream builder;
        builder << "Expected " << exVal_ << ", received " << value << std::endl;
        prom_.setError(builder.str());
      }
      else if (!--remainingCalls_)
        prom_.setValue(value);
      else if (remainingCalls_ < 0 && ov_)
        *ov_ = true;
    }

    boost::function<void(int)> tracked()
    {
      return qi::track([this](int v){ (*this)(v); }, &trackable);
    }

    struct Tracked : qi::Trackable<Tracked>
    {
      ~Tracked() { destroy(); }
    } trackable;
    qi::Promise<int> prom_;
    int exVal_;
    int remainingCalls_;
    bool* ov_;
    boost::mutex moutecks_;
  };

  TEST_F(TestGateway, testSimpleMethodCallGwService)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();

    serviceHost->listen("tcp://localhost:0");
    serviceHost->registerService("my_service", makeBaseService());
    qi::AnyObject service = client->service("my_service").value();
    int value = randomValue();

    ASSERT_EQ(service.call<int>("echoValue", value), value);
    client->close();
    serviceHost->close();
  }

  TEST_F(TestGateway, testSimpleSignalGwService)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();
    qi::Promise<int> sync;
    qi::Future<int> fut = sync.future();

    serviceHost->listen("tcp://localhost:0");
    serviceHost->registerService("my_service", makeBaseService());

    qi::AnyObject service = client->service("my_service").value();
    int value = randomValue();
    Callsync callsync(sync, value);
    service.connect("echoSignal", callsync.tracked());
    service.post("echoSignal", value);
    fut.wait();

    ASSERT_FALSE(fut.hasError());
  }

  static void serviceRegistered(unsigned int id, qi::Promise<int> prom)
  {
    prom.setValue(id);
  }

  TEST_F(TestGateway, testSDLocalService)
  {
    SessionPtr client = connectClientToGw();
    SessionPtr serviceHost = connectClientToSd();

    {
      qi::Promise<int> sync;
      serviceHost->serviceRegistered.connect(&serviceRegistered, _1, sync);
      serviceHost->listen("tcp://localhost:0");
      serviceHost->registerService("my_service", makeBaseService());
      sync.future().wait();
    }

    qi::AnyObject service;
    const int value = randomValue();
    {
      qi::Promise<int> sync;
      auto fut = sync.future();
      ASSERT_TRUE(test::finishesWithValue(client->waitForService( "my_service" )));
      service = client->service("my_service").value();
      Callsync callsync(sync, value);
      service.connect("echoSignal", callsync.tracked());
      service.post("echoSignal", value);
      fut.wait();
      ASSERT_FALSE(fut.hasError());
    }

    int res = service.call<int>("echoValue", value);
    ASSERT_EQ(res, value);
  }

  TEST_F(TestGateway, testNoSuchService)
  {
    SessionPtr client = connectClientToGw();
    SessionPtr serviceHost = connectClientToSd();

    qi::AnyObject service;
    ASSERT_ANY_THROW(service = client->service("my_service").value());

    serviceHost->listen("tcp://localhost:0");
    const auto id = serviceHost->registerService("my_service", makeBaseService()).value();
    ASSERT_TRUE(test::finishesWithValue(client->waitForService("my_service")));
    service = client->service("my_service").value();
    ASSERT_EQ(service.call<int>("echoValue", 44), 44);
    serviceHost->unregisterService(id);
    ASSERT_ANY_THROW(service.call<int>("echoValue", 44));
  }

  TEST_F(TestGateway, testSignalsProperlyDisconnected)
  {
    SessionPtr client = connectClientToGw();
    SessionPtr serviceHost = connectClientToGw();
    int value = randomValue();

    serviceHost->listen("tcp://localhost:0");
    serviceHost->registerService("my_service", makeBaseService());
    qi::AnyObject service = client->service("my_service").value();

    Callsync callsync(qi::Promise<int>(), value, 1);
    qi::Future<int> fut = callsync.prom_.future();
    qi::SignalLink callsyncOnEchoLink =
      service.connect("echoSignal", callsync.tracked()).value();
    ASSERT_TRUE(qi::isValidSignalLink(callsyncOnEchoLink));
    service.post("echoSignal", value);
    fut.wait();
    ASSERT_FALSE(fut.hasError());
    ASSERT_EQ(callsync.remainingCalls_, 0);

    // Disconnect the signal and check we don't receive it anymore
    // fut ensures we still receive the signal properly
    service.disconnect(callsyncOnEchoLink);
    callsync.prom_ = qi::Promise<int>();
    callsync.remainingCalls_ = 1;

    qi::Promise<int> witnessPromise;
    fut = witnessPromise.future();
    qi::SignalLink setValueOnEchoLink =
        service.connect("echoSignal", [&](int v){ witnessPromise.setValue(v); }).value();
    ASSERT_TRUE(qi::isValidSignalLink(setValueOnEchoLink));

    service.post("echoSignal", value);
    fut.wait();
    service.disconnect(setValueOnEchoLink);
    ASSERT_EQ(callsync.remainingCalls_, 1);

    // Reconnect the signal, disconnect the client, reconnect the client,
    // trigger the signal : we should receive it only once (links properly
    // disconnected GW-side).
    callsync.remainingCalls_ = 2;
    fut = callsync.prom_.future();
    service.connect("echoSignal", callsync.tracked());
    client->close();
    client = connectClientToGw();
    service = client->service("my_service").value();
    service.connect("echoSignal", callsync.tracked());
    service.post("echoSignal", value);
    service.post("echoSignal", value);
    fut.wait();
    ASSERT_EQ(callsync.remainingCalls_, 0);
    ASSERT_FALSE(fut.hasError());
  }

  TEST_F(TestGateway, testFunctionMultiUser)
  {
    SessionPtr serviceHost = connectClientToGw();
    SessionPtr clients[5] = {};
    qi::AnyObject serviceObjects[5] = {};
    int value = randomValue();

    serviceHost->listen("tcp://localhost:0");
    serviceHost->registerService("my_service", makeBaseService());
    for (int i = 0; i < 5; ++i)
      clients[i] = connectClientToGw();
    for (int i = 0; i < 5; ++i)
      serviceObjects[i] = clients[i]->service("my_service").value();
    for (int i = 0; i < 5; ++i)
      ASSERT_EQ(serviceObjects[i].call<int>("echoValue", value), value);
    for (int i = 0; i < 5; ++i)
      clients[i]->close();
    serviceHost->close();
  }

  TEST_F(TestGateway, testSignalsMultiUser)
  {
    SessionPtr serviceHost = connectClientToGw();
    SessionPtr clients[5] = {};
    qi::AnyObject serviceObjects[5] = {};
    qi::Promise<int> prom;
    qi::Future<int> fut = prom.future();
    int value = randomValue();
    bool overflow = false;
    Callsync callsync(prom, value, 5, &overflow);

    serviceHost->listen("tcp://localhost:0");
    serviceHost->registerService("my_service", makeBaseService());
    for (int i = 0; i < 5; ++i)
      clients[i] = connectClientToGw();
    for (int i = 0; i < 5; ++i)
      serviceObjects[i] = clients[i]->service("my_service").value();
    for (int i = 0; i < 5; ++i)
      serviceObjects[i].connect("echoSignal", callsync.tracked());
    serviceObjects[0].post("echoSignal", value);

    fut.wait();
    ASSERT_FALSE(fut.hasError());
    ASSERT_FALSE(overflow);
    ASSERT_EQ(callsync.remainingCalls_, 0);
    for (int i = 0; i <5 ; ++i)
      clients[i]->close();
  }

  void setPromiseIfCountEquals(qi::Promise<void> prom, std::atomic<int>& count, int value)
  {
    if (++count == value)
    {
      prom.setValue(nullptr);
    }
  }

  TEST_F(TestGateway, testOnSDDeathGwReconnectsAndStillWorksProperly)
  {
    SessionPtr serviceHost = connectClientToGw();
    SessionPtr client = connectClientToGw();
    auto nextSD = qi::makeSession();
    std::atomic<int> count{ 0 };
    qi::AnyObject service;
    qi::Url origUrl = sd_->url();

    qi::Promise<void> sync;
    qi::SignalLink shl = serviceHost->disconnected.connect(setPromiseIfCountEquals, sync, std::ref(count), 2);
    ASSERT_TRUE(qi::isValidSignalLink(shl));
    qi::SignalLink cl = client->disconnected.connect(setPromiseIfCountEquals, sync, std::ref(count), 2);
    ASSERT_TRUE(qi::isValidSignalLink(cl));
    serviceHost->listen("tcp://localhost:0");
    serviceHost->registerService("my_service", makeBaseService());
    service = client->service("my_service").value();
    int value = randomValue();

    ASSERT_EQ(service.call<int>("echoValue", value), value);
    sd_->close();
    sync.future().wait();

    {
      qi::Promise<void> sync;
      gw_->status().connect([sync](const qi::Gateway::Status& status) mutable {
        if (status == qi::Gateway::Status::Ready)
          sync.setValue(nullptr);
      });
      nextSD->listenStandalone(origUrl);
      sync.future().wait();
    }

    const auto gatewayEndpoints = gw_->endpoints();
    ASSERT_FALSE(gatewayEndpoints.empty());
    const auto& firstEndpoint = gatewayEndpoints[0];
    serviceHost->connect(firstEndpoint);
    client->connect(firstEndpoint);
    serviceHost->listen("tcp://localhost:0");
    serviceHost->registerService("my_service", makeBaseService());
    service = client->service("my_service").value();
    ASSERT_EQ(service.call<int>("echoValue", value), value);
    serviceHost->disconnected.disconnect(shl);
    client->disconnected.disconnect(cl);
    client->close();
    serviceHost->close();
  }

  TEST_F(TestGateway, testUnregisterSignal)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();
    qi::Promise<int> sync;
    qi::Future<int> fut = sync.future();

    serviceHost->listen("tcp://localhost:0");
    serviceHost->registerService("my_service", makeBaseService());
    qi::AnyObject service = client->service("my_service").value();
    int value = randomValue();
    {
      Callsync callsync(sync, value);
      const auto link = service.connect("echoSignal", callsync.tracked()).value();
      ASSERT_TRUE(qi::isValidSignalLink(link));
      service.post("echoSignal", value);
      fut.wait();
      ASSERT_FALSE(fut.hasError());
      qi::Future<void> fut2 = service.disconnect(link);
      ASSERT_FALSE(fut2.hasError());
    }

    {
      sync = qi::Promise<int>();
      Callsync callsync(sync, value);
      const auto link = service.connect("echoSignal", callsync.tracked()).value();
      ASSERT_TRUE(qi::isValidSignalLink(link));
      service.post("echoSignal", value);
      fut.wait();
      ASSERT_FALSE(fut.hasError());
    }
  }

  TEST_F(TestGateway, testDanglingObjectsClientService)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();

    serviceHost->listen("tcp://localhost:0");
    serviceHost->registerService("my_service", makeBaseService());
    qi::AnyObject service = client->service("my_service").value();
    qi::AnyObject danglingObject = service.call<qi::AnyObject>("getObject");


    // Test Call
    int value = randomValue();
    int tentative = danglingObject.call<int>("echoValue", value);
    ASSERT_EQ(tentative, value);

    // TestSignals
    {
      qi::Promise<int> sync;
      const auto fut = sync.future();
      const auto value = randomValue();
      Callsync callsync(sync, value);
      qi::SignalLink link = danglingObject.connect("echoSignal", callsync.tracked()).value();
      ASSERT_TRUE(qi::isValidSignalLink(link));
      danglingObject.post("echoSignal", value);
      fut.wait();
      ASSERT_FALSE(fut.hasError());
      qi::Future<void> fut2 = danglingObject.disconnect(link);
      ASSERT_FALSE(fut2.hasError());
    }
    {
      qi::Promise<int> sync;
      const auto fut = sync.future();
      const auto value = randomValue();
      Callsync callsync(sync, value);
      const auto link = danglingObject.connect("echoSignal", callsync.tracked()).value();
      ASSERT_TRUE(qi::isValidSignalLink(link));
      danglingObject.post("echoSignal", value);
      fut.wait();
      ASSERT_FALSE(fut.hasError());
    }
  }

  // In this test, the client is handing an object to the service.

  class ObjectUserService
  {
  public:
    void supplyObject(qi::AnyObject obj);
    qi::AnyObject getSuppliedObject();

  private:
    qi::AnyObject clientSuppliedObject;
  };
  void ObjectUserService::supplyObject(qi::AnyObject obj)
  {
    clientSuppliedObject = obj;
  }
  qi::AnyObject ObjectUserService::getSuppliedObject()
  {
    return clientSuppliedObject;
  }
  QI_REGISTER_OBJECT(ObjectUserService, supplyObject);

  TEST_F(TestGateway, testDanglingObjectsServiceClient)
  {
    qi::SessionPtr client = connectClientToGw();
    qi::SessionPtr serviceHost = connectClientToGw();
    qi::Object<ObjectUserService> concreteService(new ObjectUserService);

    serviceHost->listen("tcp://localhost:0");
    serviceHost->registerService("my_service", concreteService);
    qi::AnyObject service = client->service("my_service").value();
    qi::AnyObject clientHostedObject = makeBaseService();
    service.call<void>("supplyObject", clientHostedObject);
    qi::AnyObject danglingObject = concreteService->getSuppliedObject();

    // Test Call
    int value = randomValue();
    int tentative = danglingObject.call<int>("echoValue", value);
    ASSERT_EQ(tentative, value);


    // TestSignals
    {
      qi::Promise<int> sync;
      qi::Future<int> fut = sync.future();
      const auto value = randomValue();
      Callsync callsync(sync, value);
      const auto link = danglingObject.connect("echoSignal", callsync.tracked()).value();
      danglingObject.post("echoSignal", value);
      fut.wait();
      ASSERT_FALSE(fut.hasError());
      qi::Future<void> fut2 = danglingObject.disconnect(link);
      ASSERT_FALSE(fut2.hasError());
    }

    {
      qi::Promise<int> sync;
      qi::Future<int> fut = sync.future();
      const auto value = randomValue();
      Callsync callsync(sync, value);
      danglingObject.connect("echoSignal", callsync.tracked()).value();
      danglingObject.post("echoSignal", value);
      fut.wait();
      ASSERT_FALSE(fut.hasError());
    }
  }

  TEST_F(TestGateway, RegisterServiceOnGWRegistersItOnSD)
  {
    auto gwServer = connectClientToGw();
    auto sdClient = connectClientToSd();

    const auto serviceName = "my_service";
    qi::Object<ObjectUserService> concreteService(boost::make_shared<ObjectUserService>());
    gwServer->listen("tcp://localhost:0");
    gwServer->registerService(serviceName, concreteService);

    qi::AnyObject serviceObject;
    ASSERT_TRUE(test::finishesWithValue(sdClient->waitForService(serviceName)));
    ASSERT_TRUE(test::finishesWithValue(sdClient->service(serviceName),
                                        test::willAssignValue(serviceObject)));
    ASSERT_TRUE(serviceObject.isValid());

    // TODO: It would be good if this worked but right now these two objects don't have the same
    // ObjectUid.
    // ASSERT_EQ(concreteService, serviceObject);
  }

  TEST_F(TestGateway, ServiceRegisteredOnGWIsAvailableOnGW)
  {
    auto gwServer = connectClientToGw();
    auto gwClient = connectClientToGw();

    const auto serviceName = "my_service";
    qi::Object<ObjectUserService> concreteService(boost::make_shared<ObjectUserService>());
    gwServer->listen("tcp://localhost:0");
    gwServer->registerService(serviceName, concreteService);

    qi::AnyObject serviceObject;
    ASSERT_TRUE(test::finishesWithValue(gwClient->waitForService(serviceName)));
    ASSERT_TRUE(test::finishesWithValue(gwClient->service(serviceName),
                                        test::willAssignValue(serviceObject)));
    ASSERT_TRUE(serviceObject.isValid());

    // TODO: It would be good if this worked but right now these two objects don't have the same
    // ObjectUid.
    // ASSERT_EQ(concreteService, serviceObject);
  }

  TEST(TestGatewayLateSD, AttachesToSDWhenAvailable)
  {
    qi::Gateway::Config cfg;
    cfg.serviceDirectoryUrl = "tcp://127.0.0.1:59345";
    const auto gwFut = qi::Gateway::create(std::move(cfg));

    const qi::Seconds maxConnectionDelay { 2 };
    ASSERT_TRUE(test::isStillRunning(gwFut, test::willDoNothing(), maxConnectionDelay));

    auto sd = qi::makeSession();
    sd->listenStandalone("tcp://127.0.0.1:59345");

    ASSERT_TRUE(test::finishesWithValue(gwFut, test::willDoNothing(), maxConnectionDelay));
  }

  TEST(TestGatewayCreation, CancelFuture)
  {
    qi::Gateway::Config cfg;
    cfg.serviceDirectoryUrl = "tcp://127.0.0.1:59345";
    cfg.listenUrls = { "tcp://127.0.0.1:0" };
    auto gwFut = qi::Gateway::create(std::move(cfg));
    ASSERT_TRUE(test::isStillRunning(gwFut));
    gwFut.cancel();
    ASSERT_TRUE(test::finishesAsCanceled(gwFut));
  }
}
