#include <string>
#include <qi/os.hpp>

class ScopedProcess
{
public:
  ScopedProcess(
      const std::string& executable,
      const std::vector<std::string>& arguments = std::vector<std::string>())
    : _executable(executable)
  {
    char** cArgs = new char*[arguments.size()+2];
    cArgs[0] = (char*)_executable.c_str();
    for(size_t i = 0; i < arguments.size(); ++i)
      cArgs[i+1] = (char*)arguments[i].c_str();
    cArgs[arguments.size()+1] = NULL;
    _pid = qi::os::spawnvp(cArgs);
    if (_pid <= 0)
      throw std::runtime_error(
          std::string("Could not start: ") + _executable);
  }

  ~ScopedProcess()
  {
    qi::os::kill(_pid, SIGKILL);
    int status = 0;
    int ret = qi::os::waitpid(_pid, &status);
    std::cout << "Waiting for " << _executable << " has yielded the status "
                << status << " and returned " << ret << std::endl;
  }

  int pid() const
  {
    return _pid;
  }

private:
  std::string _executable;
  int _pid;
};
