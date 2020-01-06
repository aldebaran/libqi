#include <qi/applicationsession.hpp>
#include <testssl/testssl.hpp>

int main(int argc, char** argv)
{
  qi::ApplicationSession::Config cfg;
  qi::Session::Config sessionCfg;
  sessionCfg.serverSslConfig = test::ssl::serverConfig(test::ssl::server(),
                                                       test::ssl::rootCA());
  cfg.setSessionConfig(sessionCfg);
  qi::ApplicationSession app{argc, argv, cfg};
  app.run();
}
