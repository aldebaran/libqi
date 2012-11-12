/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#ifndef _TESTS_LIBTESTSESSION_TRAFICGENERATOR_HPP_
#define _TESTS_LIBTESTSESSION_TRAFICGENERATOR_HPP_

#include <vector>
#include <map>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <qimessaging/session.hpp>

void __chaosThread(void *data);

struct Behavior
{
  qi::Session  *session;
  std::string   service;
  boost::mutex *mutex;
};

class TraficGenerator
{
public:
  TraficGenerator();
  ~TraficGenerator();

public:
  bool generateCommonTrafic(const std::vector<qi::Session*> &sessions, const std::string &serviceName);
  bool generateSpam(std::vector<qi::Session *> &sessions);

  bool stopTrafic();

public:
  std::map<boost::mutex *, boost::thread *> _clients;
};

#endif // !_TESTS_LIBTESTSESSION_TRAFICGENERATOR_HPP_
