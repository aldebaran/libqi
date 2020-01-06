/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

/*!
 * \internal
 * \class SessionInitializer
 * \brief Initialize elements (Gateways, sessions, services...) needed to suit required test mode.
 * \since 1.18
 * \author Pierre Roullon
 */

#ifndef _TESTS_LIBTESTSESSION_SESSIONINITIALIZER_HPP_
#define _TESTS_LIBTESTSESSION_SESSIONINITIALIZER_HPP_

#include <map>
#include <testsession/testsession.hpp>

#include "populationgenerator.hpp"
#include "trafficgenerator.hpp"

class SessionInitializer
{
public:
  SessionInitializer();
  ~SessionInitializer();

public:
  qi::SessionPtr setUp(const std::string &serviceDirectoryUrl, TestMode::Mode mode, bool listen);
  void tearDown(qi::SessionPtr session, TestMode::Mode mode);

private:
  qi::SessionPtr setUpSD(const std::string &serviceDirectoryUrl);
  qi::SessionPtr setUpSSL(const std::string &serviceDirectoryUrl);
  qi::SessionPtr setUpGateway(const std::string &serviceDirectoryUrl);
  qi::SessionPtr setUpNightmare(const std::string &serviceDirectoryUrl);
  void tearDownSD(qi::SessionPtr session);
  void tearDownNightmare(qi::SessionPtr session);

private:
  using setUpFcnt = qi::SessionPtr (SessionInitializer::*)(const std::string &serviceDirectoryUrl);
  using tearDownFcnt = void (SessionInitializer::*)(qi::SessionPtr session);

  bool                                     _listen;

  std::map<TestMode::Mode, setUpFcnt>      _setUps;
  std::map<TestMode::Mode, tearDownFcnt>   _tearDowns;

  PopulationGenerator                     *_populationGenerator;
  TrafficGenerator                        *_trafficGenerator;
};

#endif // !_TESTS_LIBTESTSESSION_SESSIONINITIALIZER_HPP_
