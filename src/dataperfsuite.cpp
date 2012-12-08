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
  DataPerfSuite::DataPerfSuite(const std::string& projectName, const std::string& executableName, OutputData outputData, const std::string& filename)
    :_p(new DataPerfSuitePrivate)
  {
    _p->projectName = projectName;
    _p->executableName = executableName;
    _p->outputData = outputData;

    if (!filename.empty() && outputData != OutputData_None) {
      _p->out.open(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
      if (!(_p->out.is_open())) {
        std::cerr << "Can't open file " << filename << "." << std::endl
                  << "Using stdout instead." << std::endl;
      }
    }

    if (_p->out.is_open()) {
      _p->out  << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl
               << "<perf_results project=\"" << projectName << "\" executable=\""
               << executableName << "\">" << std::endl;
    }

    {
      std::cout << projectName << ": " << executableName << std::endl
                << "Name: bytes, msg/s, MB/s, period (us), cpu/total" << std::endl;
    }
  }

  DataPerfSuite::~DataPerfSuite()
  {
    close();

    delete _p;
  }

  DataPerfSuite& DataPerfSuite::operator<<(const DataPerf& data) {
    if (_p->out.is_open()) {
      if (_p->outputData & OutputData_Cpu) {
        _p->out << "\t<perf_result benchmark=\"" << data.getBenchmarkName() << "_Cpu"
                << "\" result_value=\"" << std::fixed << std::setprecision(6)
                << data.getCpu() << "\" />" << std::endl;
      }
      if (_p->outputData & OutputData_Period) {
        _p->out << "\t<perf_result benchmark=\"" << data.getBenchmarkName() << "_Period"
                << "\" result_value=\"" << std::fixed << std::setprecision(6)
                << data.getPeriod() << "\" />" << std::endl;
      }
      if (_p->outputData & OutputData_MsgPerSecond) {
        _p->out << "\t<perf_result benchmark=\"" << data.getBenchmarkName() << "_MsgPerSecond"
                << "\" result_value=\"" << std::fixed << std::setprecision(6)
                << data.getMsgPerSecond() << "\" />" << std::endl;
      }
      if (_p->outputData & OutputData_MsgMBPerSecond) {
        _p->out << "\t<perf_result benchmark=\"" << data.getBenchmarkName() << "_MsgMBPerSecond"
                << "\" result_value=\"" << std::fixed << std::setprecision(6)
                << data.getMegaBytePerSecond() << "\" />" << std::endl;
      }
    }

    {
      std::cout << data.getBenchmarkName() << ": ";
      if (data.getMsgSize() > 0) {
        std::cout
          << std::fixed << std::setprecision(2) << data.getMsgSize() << " b, "
          << data.getMsgPerSecond() << " msg/s, "
          << std::setprecision(12) << data.getMegaBytePerSecond() << " MB/s, "
          << std::setprecision(0) << data.getPeriod() << " us, "
          << std::setprecision(1) << data.getCpu() << " %"
          << std::endl;
      } else {
        std::cout
          << std::setprecision(12) << data.getMsgPerSecond() << " msg/s, "
          << data.getCpu() << " %"
          << std::endl;
      }
    }

    return *this;
  }

  void DataPerfSuite::flush()
  {
    if (_p->out.is_open())
      _p->out.flush();
    else
      std::cout.flush();
  }

  void DataPerfSuite::close()
  {
    if (_p->out.is_open()) {
      _p->out << "</perf_results>" << std::endl;
    }

    flush();

    if (_p->out.is_open())
      _p->out.close();
  }
}

