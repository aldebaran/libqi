#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <qi/log.hpp>

int main(int argc, char **argv)
{
  qiFatal("%d\n", 42);
  qiError("%d\n", 42);
  qiWarning("%d\n", 42);
  qiInfo("%d\n", 42);
  qiDebug("%d\n", 42);

  qisFatal   << "f" << 42 << std::endl;
  qisError   << "e" << 42 << std::endl;
  qisWarning << "w" << 42 << std::endl;
  qisInfo    << "i" << 42 << std::endl;
  qisDebug   << "d" << 42 << std::endl;
}
