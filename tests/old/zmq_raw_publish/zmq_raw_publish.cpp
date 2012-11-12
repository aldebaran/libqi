/**
** Copyright (C) 2010 Aldebaran Robotics
*/

#include <vector>
#include <iostream>
#include <cmath>
#include <qimessaging/perf/dataperftimer.hpp>
#include <zmq.hpp>

using qi::perf::DataPerfTimer;

static const int gThreadCount = 10;
static const int gLoopCount   = 10000;

int main(int argc, char **argv)
{
  //  Initialise 0MQ infrastructure.
  zmq::context_t ctx (1);
  zmq::socket_t s (ctx, ZMQ_PUB);
  s.bind("tcp://127.0.0.1:5555");
  sleep(5);
  for (int i = 0; i < 12; ++i)
  {
    unsigned int                  numBytes = (unsigned int)pow(2.0f,(int)i);
    std::string                   request = std::string(numBytes, 'B');

    zmq::message_t msg (request.size());
    memcpy (msg.data (), request.data(), msg.size ());
    std::cout << "Sending at size " << numBytes << "...";
    for (int j = 0; j< gLoopCount; ++j)
    {
      //std::cout << request.data() << std::endl;
      s.send (msg);
    }
    std::cout << " Done." << std::endl;
  }
  std::cout << "All Done." << std::endl;
  return 0;
}
