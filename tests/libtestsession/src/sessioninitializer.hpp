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
#include "traficgenerator.hpp"

class SessionInitializer
{
public:
  SessionInitializer();
  ~SessionInitializer();

public:
  bool setUp(qi::Session *session, const std::string &serviceDirectoryUrl, TestMode::Mode mode, bool listen);
  bool tearDown(qi::Session *session, TestMode::Mode mode);

private:
  bool setUpSD(qi::Session *session, const std::string &serviceDirectoryUrl);
  bool setUpSSL(qi::Session *session, const std::string &serviceDirectoryUrl);
  bool setUpNightmare(qi::Session *session, const std::string &serviceDirectoryUrl);
  bool tearDownSD(qi::Session *session);
  bool tearDownNightmare(qi::Session *session);

private:
  typedef bool (SessionInitializer::*setUpFcnt)(qi::Session *session, const std::string &serviceDirectoryUrl);
  typedef bool (SessionInitializer::*tearDownFcnt)(qi::Session *session);

  bool                                     _listen;

  std::map<TestMode::Mode, setUpFcnt>      _setUps;
  std::map<TestMode::Mode, tearDownFcnt>   _tearDowns;

  PopulationGenerator                     *_populationGenerator;
  TraficGenerator                         *_traficGenerator;
};

#endif // !_TESTS_LIBTESTSESSION_SESSIONINITIALIZER_HPP_
