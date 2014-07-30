/*
*  Author(s):
*  - Nicolas Cornu <ncornu@aldebaran-robotics.com>
*
*  Copyright (C) 2012-2013 Aldebaran Robotics
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

    std::cout << projectName << ": " << executableName << std::endl
              << "Name: bytes, msg/s, MB/s, period (us), cpu/total" << std::endl;
  }

  DataPerfSuite::~DataPerfSuite()
  {
    close();

    delete _p;
  }

  DataPerfSuite& DataPerfSuite::operator<<(const DataPerf& data) {
    if (_p->out.is_open()) {
      std::string resultType;
      float resultValue;
      switch (_p->outputData)
      {
      case OutputData_Cpu:
        resultType = "Cpu";
        resultValue = data.getCpu();
        break;
      case OutputData_Period:
        resultType = "Period";
        resultValue = data.getPeriod();
        break;
      case OutputData_MsgPerSecond:
        resultType = "MsgPerSecond";
        resultValue = data.getMsgPerSecond();
        break;
      case OutputData_MsgMBPerSecond:
      default:
        resultType = "MsgMBPerSecond";
        resultValue = data.getMegaBytePerSecond();
        break;
      }
      _p->out << "\t<perf_result "
              << "benchmark=\"" << data.getBenchmarkName() << "_" << resultType << "\" "
              << "result_value=\"" << std::fixed << std::setprecision(6) << resultValue << "\" "
              << "result_type=\"" << resultType << "\" "
              << "test_name=\"" << data.getBenchmarkName() << "\" ";
      if (data.getVariable() != "")
        _p->out << "result_variable=\"" << data.getVariable() << "\" ";
      else if (data.getMsgSize() != 0)
        _p->out << "result_variable=\"" << data.getMsgSize() << "\" ";
      _p->out << "/>" << std::endl;
    }


    std::cout << data.getBenchmarkName() << "-" << data.getVariable() << ": ";
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
          << data.getPeriod() << " us, "
          << data.getCpu() << " %"
          << std::endl;
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
