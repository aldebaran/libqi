/*
** Author(s):
** - Nicolas Cornu <ncornu@aldebaran-robotics.com>
**
** Copyright (C) 2012-2013 Aldebaran Robotics
*/

#include "dataperf_p.hpp"

namespace qi
{
  DataPerfPrivate::DataPerfPrivate()
    : benchmarkName(""), wallClockElapsed(0), cpuElapsed(0), fLoopCount(0), fMsgSize(0)
  { }

  DataPerf::DataPerf()
    : _p(new DataPerfPrivate)
  { }

  DataPerf::~DataPerf()
  {
    delete _p;
  }

  void DataPerf::start(const std::string& benchmarkName, unsigned long loopCount, unsigned long msgSize)
  {
    _p->benchmarkName = benchmarkName;
    _p->fLoopCount = loopCount;
    _p->fMsgSize = msgSize;
    _p->cpuTime.restart();
    qi::os::gettimeofday(&(_p->fStartTime));
  }

  void DataPerf::stop() {
    qi::os::timeval tv;

    qi::os::gettimeofday(&tv);
    _p->cpuElapsed = _p->cpuTime.elapsed();

    _p->wallClockElapsed  = (double)(tv.tv_sec - _p->fStartTime.tv_sec);
    _p->wallClockElapsed += (double)(tv.tv_usec - _p->fStartTime.tv_usec) / 1000 / 1000;
  }

  std::string DataPerf::getBenchmarkName() const
  {
    return _p->benchmarkName;
  }

  unsigned long DataPerf::getMsgSize() const
  {
    return _p->fMsgSize;
  }

  double DataPerf::getPeriod() const
  {
    return (double)(_p->wallClockElapsed) * 1000.0 * 1000.0 / (_p->fLoopCount);
  }

  double DataPerf::getCpu() const
  {
    return (double)(_p->cpuElapsed) / (double)(_p->wallClockElapsed) * 100;
  }

  double DataPerf::getMsgPerSecond() const
  {
    return 1.0 / ((_p->wallClockElapsed) / (1.0 * (_p->fLoopCount)));
  }

  double DataPerf::getMegaBytePerSecond() const
  {
    if (_p->fMsgSize > 0)
      return (getMsgPerSecond() * _p->fMsgSize) / (1024.0 * 1024.0);

    return -1;
  }
}

