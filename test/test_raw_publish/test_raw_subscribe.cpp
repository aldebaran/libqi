/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <vector>
#include <iostream>
#include <alcommon-ng/tools/dataperftimer.hpp>
#include <alcommon-ng/tools/sleep.hpp>
#include <zmq.hpp>

using AL::Test::DataPerfTimer;

static const int gThreadCount = 10;
static const int gLoopCount   = 10000;

int main(int argc, char **argv)
{
  //  Initialise 0MQ infrastructure.
  zmq::context_t ctx (1);
  zmq::socket_t s (ctx, ZMQ_SUB);
  //s.setsockopt (zmq::socket_t::SUBSCRIBE, "")
  s.setsockopt(ZMQ_SUBSCRIBE, "", 0);

  s.connect ("tcp://127.0.0.1:5555");

  DataPerfTimer dt("Subscribe string");
  for (int i = 0; i < 12; ++i)
  {
    unsigned int                  numBytes = (unsigned int)pow(2.0f,(int)i);
    zmq::message_t msg;

    dt.start(gLoopCount, numBytes);
    for (int j = 0; j< gLoopCount; ++j)
    {
      //zmq::message_t msg;
      //std::cout << "ready" << std::endl;
      s.recv (&msg);
      //std::cout << "received 1" << std::endl;
    }
    dt.stop();
  }
  return 0;
}
