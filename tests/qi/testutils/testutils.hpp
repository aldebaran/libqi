#pragma once

#include <memory>
#include <string>
#include <vector>
#include <boost/range/algorithm/transform.hpp>
#include <cstring>
#include <qi/os.hpp>

namespace test
{
  /// Deleter for pointers allocated with malloc.
  /// Can be used with unique_ptr.
  template<typename T>
  struct Free
  {
      using value_type = T;

      /// Precondition: p was allocated with malloc
      /// Precondition: p was not already freed
      void operator()(T* p) const
      {
        free(p);
      }
  };

  /// Runs a process from construction to destruction.
  /// At destruction, the process is killed with SIGKILL.
  class ScopedProcess
  {
    using Strings = std::vector<std::string>;
  public:
    explicit ScopedProcess(const std::string& executable, const Strings& arguments = Strings{})
      : _executable(executable), _pid{}
    {
      // spawnvp expects an array of pointers on non-const null-terminated strings.
      // Unfortunately, there's no way to get a non-const null-terminated string
      // out of a std::string. So we need to copy the std::string to char*.
      // 2 vectors are used:
      //  1 for unique_ptrs to avoid leaks
      //  1 for the char* themselves, whose memory will be passed to spawnvp
      using namespace std;
      using Ptr = unique_ptr<char, Free<char>>;
      vector<Ptr> uptr_args(arguments.size() + 2);
      uptr_args[0].reset(strdup(_executable.c_str()));
      boost::transform(arguments, uptr_args.begin() + 1, [](const string& s) {
        return Ptr(strdup(s.c_str()));
      });
      // Last unique_ptr is default constructed, so its underlying pointer is null.
      vector<char*> c_args(uptr_args.size(), nullptr);
      boost::transform(uptr_args, c_args.begin(), [](const Ptr& p) {
        return p.get();
      });
      // Vector's memory is contiguous, so it's ok to pass its memory to spawnvp.
      _pid = qi::os::spawnvp(&c_args[0]);
      if (_pid <= 0)
        throw runtime_error("Could not start: " + _executable);
    }

    ~ScopedProcess()
    {
      if (qi::os::kill(_pid, SIGKILL) != 0) {
        std::cout << "Failed to send a kill signal to " << _executable << std::endl;
      }
      int status = 0;
      int ret = qi::os::waitpid(_pid, &status);
      std::cout << "Waited for " << _executable << " : return the status "
                  << status << " and returned " << ret << std::endl;
    }

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
