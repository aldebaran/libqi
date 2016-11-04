/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "defaultservice.hpp"
#include "populationgenerator.hpp"
#include "sessioninitializer.hpp"

static int defaultTimeoutMs = 1000;

SessionInitializer::SessionInitializer() :
  _listen(false),
  _populationGenerator(0),
  _trafficGenerator(0)
{
  _setUps[TestMode::Mode_SD] = &SessionInitializer::setUpSD;
  _setUps[TestMode::Mode_SSL] = &SessionInitializer::setUpSSL;
  _setUps[TestMode::Mode_Direct] = &SessionInitializer::setUpSD;
  _setUps[TestMode::Mode_Nightmare] = &SessionInitializer::setUpNightmare;
  _setUps[TestMode::Mode_Gateway] = &SessionInitializer::setUpSD;

  _tearDowns[TestMode::Mode_SD] = &SessionInitializer::tearDownSD;
  _tearDowns[TestMode::Mode_SSL] = &SessionInitializer::tearDownSD;
  _tearDowns[TestMode::Mode_Direct] = &SessionInitializer::tearDownSD;
  _tearDowns[TestMode::Mode_Nightmare] = &SessionInitializer::tearDownNightmare;
  _tearDowns[TestMode::Mode_Gateway] = &SessionInitializer::tearDownSD;
}

SessionInitializer::~SessionInitializer()
{
}

void SessionInitializer::setUp(qi::SessionPtr session, const std::string &serviceDirectoryUrl, TestMode::Mode mode, bool listen)
{
  if (_setUps.find(mode) == _setUps.end())
    throw TestSessionError("[Internal] setUp mode not handled.");

  _listen = listen;
  (this->*_setUps[mode])(session, serviceDirectoryUrl);
}

void SessionInitializer::tearDown(qi::SessionPtr session, TestMode::Mode mode)
{
  if (_tearDowns.find(mode) == _tearDowns.end())
    throw TestSessionError("[Internal] tearDown mode not handled.");

  (this->*_tearDowns[mode])(session);
}

void SessionInitializer::setUpSD(qi::SessionPtr session, const std::string &serviceDirectoryUrl)
{
  session->connect(serviceDirectoryUrl);
  if (_listen == true)
    session->listen("tcp://0.0.0.0:0");
}

void SessionInitializer::setUpSSL(qi::SessionPtr session, const std::string &serviceDirectoryUrl)
{
  session->connect(serviceDirectoryUrl).value(defaultTimeoutMs);

  if (_listen == true)
  {
    session->setIdentity(qi::path::findData("qi", "server.key"),
                         qi::path::findData("qi", "server.crt"));
    session->listen("tcps://0.0.0.0:0");
  }
}

void SessionInitializer::tearDownSD(qi::SessionPtr session)
{
  if (session->isConnected())
    session->close().value(defaultTimeoutMs);
}

void SessionInitializer::setUpNightmare(qi::SessionPtr session, const std::string &serviceDirectoryUrl)
{
  std::string serviceName;

  // #1 Connect session to service directory.
  session->connect(serviceDirectoryUrl).value(defaultTimeoutMs);

  // #1.1 If session is a client session, that's it.
  if (_listen == false)
    return;

  // #1.2 Make session listen.
  session->listen("tcp://0.0.0.0:0");

  // #2 Allocate population and traffic tools.
  _populationGenerator = new PopulationGenerator();
  _trafficGenerator = new TrafficGenerator();

  // #3 Generate an unique name for hidder service
  if (DefaultService::generateUniqueServiceName(serviceName) == false)
    throw TestSessionError("[Internal] Cannot generate unique service name.");

  // #4 Register hidden service.
  session->registerService(serviceName, DefaultService::getDefaultService());

  // #5 Populate with client session and generate traffic.
  if (_populationGenerator->populateClients(serviceDirectoryUrl, 10000) == false)
    throw std::runtime_error("failed to create clients to produce traffic");

  if (_trafficGenerator->generateCommonTraffic(_populationGenerator->clientPopulation(), serviceName) == false)
    throw std::runtime_error("failed to produce traffic");
}

void SessionInitializer::tearDownNightmare(qi::SessionPtr session)
{
  if (_trafficGenerator)
    _trafficGenerator->stopTraffic();

  delete _populationGenerator;
  delete _trafficGenerator;

  if (session->isConnected())
    session->close().value(defaultTimeoutMs);
}
