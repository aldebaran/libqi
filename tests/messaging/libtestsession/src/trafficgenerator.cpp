/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "trafficgenerator.hpp"
#include <chrono>
#include <thread>
#include <random>

qiLogCategory("qimessaging.testsession");

TrafficGenerator::TrafficGenerator()
{
}

TrafficGenerator::~TrafficGenerator()
{
  stopTraffic();
}

bool TrafficGenerator::generateCommonTraffic(const std::vector<qi::SessionPtr> &sessions, const std::string &serviceName)
{
  Behavior     *b;
  qi::SessionPtr sess;

  for (std::vector<qi::SessionPtr>::const_iterator it = sessions.begin(); it != sessions.end(); ++it)
  {
    b = new Behavior();
    sess = (*it);

    b->session = sess;
    b->service = serviceName;
    b->mutex = new boost::mutex;
    _clients[b->mutex] = new boost::thread(boost::bind(__chaosThread, b));
  }

  return true;
}

bool TrafficGenerator::generateSpam(std::vector<qi::SessionPtr>&)
{
  return false;
}

bool TrafficGenerator::stopTraffic()
{
  for (std::map<boost::mutex *, boost::thread *>::iterator it = _clients.begin(); it != _clients.end(); it = _clients.begin())
  {
    (*it).first->lock();
    delete (*it).second;
    delete (*it).first;
    _clients.erase(it);
  }

  return true;
}

void __chaosThread(void *data)
{
  Behavior *b = reinterpret_cast<Behavior *>(data);
  qi::SessionPtr  session;
  boost::mutex *mutex;
  std::string  service;
  std::string  methodToCall;
  std::string  serviceDirectory;
  int          nbCalls;

  // #0 First, get info and delete Behavior struct (no leak possible on thread interrupt).
  session = b->session;
  service = b->service;
  mutex = b->mutex;
  methodToCall = "ping";
  serviceDirectory = session->url().str();
  delete b;

  // #1 Init random.
  auto randEngine = [] {
    std::random_device rd;
    std::seed_seq seq{ rd(), rd(), rd(), rd(), rd(), rd(), rd(), rd() };
    return std::default_random_engine{ seq };
  }();
  std::uniform_int_distribution<int> dis0to9{ 0, 9 };
  std::uniform_int_distribution<int> dis0to49{ 0, 49 };
  const auto random0to9 = [&] { return dis0to9(randEngine); };
  const auto random0to49 = [&] { return dis0to49(randEngine); };

  // #2 Run until interrupt :
  while (mutex->try_lock())
  {
    if (session->isConnected() == false)
      session->connect(serviceDirectory).wait(1000);

    // #2.1 Get proxy on service.
    qi::AnyObject proxy = session->service(service).value();

    if (proxy.asGenericObject() == 0)
    {
      qiLogError() << "[Nightmare] Cannot get proxy on " << service << ", aborting thread.";
      mutex->unlock();
      return;
    }

    // #2.2 If there is a method to call, call it a number of time between 1 and 50.
    nbCalls = 1;
    while (methodToCall.compare("") != 0 && nbCalls > 0)
    {
      // #2.2.1 Randomise number of call.
      nbCalls = random0to49() + 1;

      // #2.2.2 Call method.
      std::string pong = proxy.call<std::string>(methodToCall);

      // #2.2.3 Sleep a random time between 0 and 9ms.
      std::this_thread::sleep_for(std::chrono::milliseconds{ random0to9() });

      nbCalls--;
    }

    // #2.3 Sleep a random time between 0 and 9ms.
    std::this_thread::sleep_for(std::chrono::milliseconds{ random0to9() });

    // #2.3 Disconnect session.
    session->close().wait(qi::Seconds{ 10 });
    mutex->unlock();
  }
}
