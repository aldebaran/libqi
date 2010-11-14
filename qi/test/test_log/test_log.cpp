#include <qi/log.hpp>

int main(int argc, char **argv)
{
  qiFatal("fatal: %d\n", 42);
  qiError("error: %d\n", 42);
  qiWarning("warning: %d\n", 42);
  qiInfo("info: %d\n", 42);
  qiDebug("debug: %d\n", 42);

  qisFatal   << "fatal" << 42 << std::endl;
  qisError   << "error" << 42 << std::endl;
  qisWarning << "warning" << 42 << std::endl;
  qisInfo    << "info" << 42 << std::endl;
  qisDebug   << "debug" << 42 << std::endl;
}
