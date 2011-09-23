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
  qiLogError("qimessaging")   << "e" << 42 << std::endl;
  qiLogWarning("qimessaging") << "w" << 42 << std::endl;
  qisInfo    << "i" << 42 << std::endl;
  qiLogDebug("qimessaging")   << "d" << 42 << std::endl;
}
