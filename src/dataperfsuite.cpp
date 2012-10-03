/*
*  Author(s):
*  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
*
*  Copyright (C) 2012 Aldebaran Robotics
*/

#include "dataperfsuite_p.hpp"

#include <iomanip>

namespace qi
{
  DataPerfSuite::DataPerfSuite(const std::string& projectName, const std::string& executableName, OutputType outputType, const std::string& filename)
    :_p(new DataPerfSuitePrivate)
  {
    _p->projectName = projectName;
    _p->executableName = executableName;
    _p->outputType = outputType;

    if (!filename.empty()) {
      _p->out.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
      if (!(_p->out.is_open())) {
        std::cerr << "Can't open file " << filename << "." << std::endl
                  << "Using stdout instead." << std::endl;
      }
    }

    switch (_p->outputType) {
      case OutputType_Codespeed:
        {
          _p->getOut() << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl
                       << "<perf_results project=\"" << projectName << "\" executable=\"" << executableName << "\">" << std::endl;
        }
        break;
      case OutputType_Normal:
        {
          _p->getOut() << projectName << ": " << executableName << std::endl
                       << "Name: bytes, msg/s, MB/s, period (us), cpu/total" << std::endl;
        }
        break;
    }
  }

  DataPerfSuite::~DataPerfSuite()
  {
    switch (_p->outputType) {
      case OutputType_Codespeed:
        {
          _p->getOut() << "</perf_results>" << std::endl;
        }
        break;
      case OutputType_Normal:
        { }
        break;
    }

    if (_p->out.is_open())
      _p->out.close();

    delete _p;
  }

  DataPerfSuite& DataPerfSuite::operator<<(const DataPerf& data) {
    switch (_p->outputType) {
      case OutputType_Codespeed:
        {
          _p->getOut() << "\t<perf_result benchmark=\"" << data.getBenchmarkName()
                       << "\" result_value=\"" << std::fixed << std::setprecision(0) << data.getPeriod() << "\" />" << std::endl;
        }
        break;

      case OutputType_Normal:
        {
          _p->getOut() << data.getBenchmarkName() << ": ";
          if (data.getMsgSize() > 0) {
            _p->getOut()
              << std::fixed << std::setprecision(2) << data.getMsgSize() << " b, "
              << data.getMsgPerSecond() << " msg/s, "
              << std::setprecision(12) << data.getMegaBytePerSecond() << " MB/s, "
              << std::setprecision(0) << data.getPeriod() << " us, "
              << std::setprecision(1) << data.getCpu() << " %"
              << std::endl;
          } else {
            _p->getOut()
              << std::setprecision(12) << data.getMsgPerSecond() << " msg/s, "
              << data.getCpu() << " %"
              << std::endl;
          }
        }
        break;
    }

    return *this;
  }

  std::ostream& DataPerfSuitePrivate::getOut()
  {
    return (out.is_open() ? out : std::cout);
  }
}

