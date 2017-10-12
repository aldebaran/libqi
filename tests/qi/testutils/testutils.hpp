#pragma once

#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <boost/config.hpp>

namespace test
{
  /// Runs a process from construction to destruction.
  /// At destruction, the process is killed with SIGKILL.
  class ScopedProcess
  {
    static const std::chrono::milliseconds defaultWaitReadyDuration;

    using Strings = std::vector<std::string>;
  public:
    explicit ScopedProcess(const std::string& executable,
                           const Strings& arguments = Strings{},
                           std::chrono::milliseconds waitReadyDuration = defaultWaitReadyDuration);

    ~ScopedProcess();

    // non-copyable
    ScopedProcess(const ScopedProcess&) = delete;
    ScopedProcess& operator=(const ScopedProcess&) = delete;

    int pid() const
    {
      return _pid;
    }

  private:
    std::string _executable;
    int _pid;
  };

} // namespace test
