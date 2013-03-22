/*
**
** Author(s):
**  - Pierre Roullon <proullon@aldebaran-robotics.com>
**
** Copyright (C) 2012 Aldebaran Robotics
*/

#include "traficgenerator.hpp"

qiLogCategory("qimessaging.testsession");

TraficGenerator::TraficGenerator()
{
}

TraficGenerator::~TraficGenerator()
{
  stopTrafic();
}

bool TraficGenerator::generateCommonTrafic(const std::vector<qi::Session *> &sessions, const std::string &serviceName)
{
  Behavior     *b;
  qi::Session  *sess;

  for (std::vector<qi::Session *>::const_iterator it = sessions.begin(); it != sessions.end(); ++it)
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

bool TraficGenerator::generateSpam(std::vector<qi::Session *> &sessions)
{
  return false;
}

bool TraficGenerator::stopTrafic()
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
  qi::Session  *session;
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
  srand(static_cast<unsigned int>(time(0)));

  // #2 Run until interrupt :
  while (mutex->try_lock())
  {
    if (session->isConnected() == false)
      session->connect(serviceDirectory).wait(1000);

    // #2.1 Get proxy on service.
    qi::ObjectPtr proxy = session->service(service);

    if (proxy.get() == 0)
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
      nbCalls = rand() % 50 + 1;

      // #2.2.2 Call method.
      std::string pong = proxy->call(methodToCall);

      // #2.2.3 Sleep a random time between 0 and 9ms.
      qi::os::msleep(rand() % 10);

      nbCalls--;
    }

    // #2.3 Sleep a random time between 0 and 9ms.
    qi::os::msleep(rand() % 10);

    // #2.3 Disconnect session.
    session->close().wait(1000);
    mutex->unlock();
  }
}
